/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#include "db_connection.h"

#include "configuration_strtpl.h"
#include "db_query.h"
#include "file_structs.h"
#include "models_configurations.h"
#include "program_state.h"
#include "Logging.h"

#include <functional>
#include <map>
#include <sstream>

#include <assert.h>


/* todo: в итоге здесь перемешаны модуль конфигурации бд,
 *   и модуль инициализации полей таблиц бд */
namespace update_configuration_functional {
typedef std::function<merror_t(db_parameters *,
    const std::string &value)> update_dbconfig_f;

merror_t update_db_dry_run(db_parameters *dbp, const std::string &val) {
  return (dbp) ? set_bool(val, &dbp->is_dry_run) : ERROR_INIT_ZERO_ST;
}
merror_t update_db_client(db_parameters *dbp, const std::string &val) {
  return (dbp) ? set_db_client(val, &dbp->supplier) : ERROR_INIT_ZERO_ST;
}
merror_t update_db_name(db_parameters *dbp, const std::string &val) {
  dbp->name = trim_str(val);
  return ERROR_SUCCESS_T;
}
merror_t update_db_username(db_parameters *dbp, const std::string &val) {
  dbp->username = trim_str(val);
  return ERROR_SUCCESS_T;
}
merror_t update_db_password(db_parameters *dbp, const std::string &val) {
  dbp->password = trim_str(val);
  return ERROR_SUCCESS_T;
}
merror_t update_db_host(db_parameters *dbp, const std::string &val) {
  dbp->host = trim_str(val);
  return ERROR_SUCCESS_T;
}
merror_t update_db_port(db_parameters *dbp, const std::string &val) {
  return (dbp) ? set_int(val, &dbp->port) : ERROR_INIT_ZERO_ST;
}

struct dbconfig_functions {
  /** \brief функция обновляющая параметр */
  update_dbconfig_f update;
  // /** \brief функция возвращающая строковые значения */
  // get_strtpl get_str_tpl;
};

static std::map<const std::string, dbconfig_functions> map_dbconfig_fuctions =
    std::map<const std::string, dbconfig_functions> {
  {STRTPL_CONFIG_DB_DRY_RUN, {update_db_dry_run}},
  {STRTPL_CONFIG_DB_CLIENT, {update_db_client}},
  {STRTPL_CONFIG_DB_NAME, {update_db_name}},
  {STRTPL_CONFIG_DB_USERNAME, {update_db_username}},
  {STRTPL_CONFIG_DB_PASSWORD, {update_db_password}},
  {STRTPL_CONFIG_DB_HOST, {update_db_host}},
  {STRTPL_CONFIG_DB_PORT, {update_db_port}}
};
}  // namespace update_configuration_functional


/* db_parameters */
namespace ns_ucf = update_configuration_functional;
db_parameters::db_parameters()
  : is_dry_run(true), supplier(db_client::NOONE) {}

merror_t db_parameters::SetConfigurationParameter(
    const std::string &param_strtpl, const std::string &param_value) {
  if (param_strtpl.empty())
    return ERROR_STRTPL_TPLNULL;
  merror_t error = ERROR_STRTPL_TPLUNDEF;
  auto it_map = ns_ucf::map_dbconfig_fuctions.find(param_strtpl);
  if (it_map != ns_ucf::map_dbconfig_fuctions.end())
    error = it_map->second.update(this, param_value);
  return error;
}

std::string db_parameters::GetInfo() const {
  std::string info = "Параметры базы данных:\n";
  if (is_dry_run)
    return info += "dummy connection\n";
  return info + db_client_to_string(supplier) + "\n\tname: " + name +
      "\n\tusername: " + username +
      "\n\thost: " + host + ":" + std::to_string(port) + "\n";
}

/* DBConnection */
DBConnection::DBConnection(const db_parameters &parameters)
  : status_(STATUS_DEFAULT), parameters_(parameters), is_connected_(false),
    is_dry_run_(true) {
  // дефайн на гугло десты БД, чтобы не линковать много объектников
#if !defined(DATABASE_TEST)
  is_dry_run_ = ProgramState::Instance().IsDryRunDBConn();
#else
  is_dry_run_ = false;
#endif  // !DATABASE_TEST
}

DBConnection::~DBConnection() {}

mstatus_t DBConnection::GetStatus() const {
  return status_;
}

merror_t DBConnection::GetErrorCode() const {
  return error_.GetErrorCode();
}

bool DBConnection::IsOpen() const {
  return is_connected_;
}

void DBConnection::LogError() {
  error_.LogIt();
}

/* setup quries text */
std::stringstream DBConnection::setupAddSavePointString(
    const db_save_point &sp) {
  std::stringstream sstr;
  sstr << "SAVEPOINT " << sp.GetString() << ";";
  return sstr;
}
std::stringstream DBConnection::setupRollbackToSavePoint(
    const db_save_point &sp) {
  std::stringstream sstr;
  sstr << "ROLLBACK TO SAVEPOINT " << sp.GetString() << ";";
  return sstr;
}
std::stringstream DBConnection::setupAddColumnString(
    const std::pair<db_table, const db_variable &> &pdv) {
  std::stringstream sstr;
  sstr << "ALTER TABLE " << get_table_name(pdv.first) << " ADD COLUMN "
      << db_variable_to_string(pdv.second) << ";";
  return sstr;
}
std::stringstream DBConnection::setupCreateTableString(
    const db_table_create_setup &fields) {
  std::stringstream sstr;
  sstr << "CREATE TABLE " << get_table_name(fields.table) << " (";
  // сначала забить все поля
  for (const auto &field : fields.fields) {
    if (!error_.GetErrorCode()) {
      sstr << db_variable_to_string(field) << ", ";
    } else {
      break;
    }
  }
  if (!error_.GetErrorCode()) {
    // UNIQUE constraint
    sstr << db_unique_constrain_to_string(fields);
    // REFERENCES
    if (!fields.ref_strings.empty()) {
      for (const auto &ref : fields.ref_strings) {
        sstr << db_reference_to_string(ref) << ", ";
        if (error_.GetErrorCode())
          break;
      }
    }
    // PRIMARY KEY
    if (!error_.GetErrorCode())
      sstr << db_primarykey_to_string(fields.pk_string);
  }
  sstr << ");";
  return sstr;
}
std::stringstream DBConnection::setupDropTableString(
    const db_table_drop_setup &drop) {
  std::stringstream sstr;
  sstr << "DROP TABLE " << get_table_name(drop.table) << " " <<
      db_reference::GetReferenceActString(drop.act) << ";";
  return sstr;
}
std::stringstream DBConnection::setupInsertString(
    const db_query_insert_setup &fields) {
  if (fields.values_vec.empty()) {
    error_.SetError(ERROR_DB_VARIABLE, "Нет данных для INSERT операции");
    return std::stringstream();
  }
  std::string fnames = "INSERT INTO " + get_table_name(fields.table) + " (";
  std::string values = "VALUES (";
  std::vector<std::string> rows(fields.values_vec.size());
  for (const auto &row: fields.values_vec) {
    for (const auto &x : row) {
      if (x.first >= 0 && x.first < fields.fields.size()) {
        fnames += std::string(fields.fields[x.first].fname) + ", ";
        values += x.second + ", ";
      } else {
        Logging::Append(io_loglvl::debug_logs, "Ошибка индекса операции INSERT.\n"
            "\tДля таблицы " + get_table_name(fields.table));
      }
    }
  }
  fnames.replace(fnames.size() - 2, fnames.size() - 1, ")");
  values.replace(values.size() - 2, values.size() - 1, ")");
  std::stringstream sstr;
  sstr << fnames << " " << values << " RETURNING ID;";
  return sstr;
}
std::stringstream DBConnection::setupDeleteString(
    const db_query_delete_setup &fields) {
  std::stringstream sstr;
  if (fields.where_condition != nullptr) {
    sstr << "DELETE FROM " << get_table_name(fields.table) <<
        " WHERE " << fields.where_condition->GetString() << ";";
  } else {
    sstr << "DELETE * FROM " << get_table_name(fields.table) << ";";
  }
  return sstr;
}
std::stringstream DBConnection::setupSelectString(
    const db_query_select_setup &fields) {
  std::stringstream sstr;
  sstr << "SELECT * FROM " << get_table_name(fields.table);
  if (fields.where_condition != nullptr)
    sstr << " WHERE " << fields.where_condition->GetString();
  sstr << ";";
  return sstr;
}
std::stringstream DBConnection::setupUpdateString(
    const db_query_update_setup &fields) {
  std::stringstream sstr;
  if (!fields.values.empty()) {
    sstr << "UPDATE " << get_table_name(fields.table)
        << " SET ";
    std::string set_str = "";
    for (const auto &x: fields.values)
      set_str += std::string(fields.fields[x.first].fname)
          + " = " + x.second + ",";
    set_str[set_str.size() - 1] = ' ';
    sstr << set_str;
    if (fields.where_condition != nullptr)
      sstr << " WHERE " << fields.where_condition->GetString();
    sstr << ";";
  }
  return sstr;
}

std::string DBConnection::db_unique_constrain_to_string(
    const db_table_create_setup &cs) {
  std::string result = "";
  for (const auto &x: cs.unique_constrains) {
    std::stringstream sstr;
    sstr << "UNIQUE(";
    for (const auto y: x)
      sstr << y << ", ";
    result += sstr.str();
    result.replace(result.size() - 2, result.size() - 1, "),");
  }
  return result;
}

std::string DBConnection::db_reference_to_string(
    const db_reference &ref) {
  std::string str;
  merror_t ew = ref.CheckYourself();
  if (!ew) {
    if (ref.is_foreign_key)
      str += "FOREIGN KEY ";
    str += "(" + ref.fname + ") REFERENCES " + get_table_name(ref.foreign_table) +
        " (" + ref.foreign_fname + ")";
    if (ref.has_on_delete)
      str += " ON DELETE " + ref.GetReferenceActString(ref.delete_method);
    if (ref.has_on_update)
      str += " ON UPDATE " + ref.GetReferenceActString(ref.update_method);
  } else {
    error_.SetError(ew,
        "Проверка поля ссылки на другую таблицу завершилось ошибкой");
  }
  return str;
}

std::string DBConnection::db_primarykey_to_string(
    const db_complex_pk &pk) {
  std::string str;
  if (!pk.fnames.empty()) {
    str = "PRIMARY KEY (";
    for (const auto &fname : pk.fnames)
      str += fname + ", ";
    str.replace(str.size() - 2, str.size() - 1, ")");
  } else {
    error_.SetError(ERROR_DB_TABLE_PKEY, "ни одного первичного ключа "
        "для таблицы не задано");
  }
  return str;
}
