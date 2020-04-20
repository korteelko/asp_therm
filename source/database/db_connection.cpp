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


namespace table_fields_setup {
/** \brief Сетап таблицы БД хранения данных о модели */
/* SQL_QUERY:
TABLE MODEL_INFO (
  model_id autoinc,
  model_type uint NOT NULL,
  model_subtype uint,
  vers_major uint not NULL,
  vers_minor uint,
  short_info string,
  PRIMARY KEY (model_id)
); */
static const db_fields_collection model_info_fields = {
  db_variable("model_id", db_type::type_autoinc,
      {.is_primary_key = true, .is_unique = true, .can_be_null = false}),
  db_variable("model_type", db_type::type_int, {.can_be_null = false}),
  db_variable("model_subtype", db_type::type_int, {}),
  db_variable("vers_major", db_type::type_int, {.can_be_null = false}),
  db_variable("vers_minor", db_type::type_int, {}),
  db_variable("short_info", db_type::type_text, {})
};
static const db_table_create_setup model_info_create_setup(
    db_table::table_model_info, model_info_fields);

/* SQL_QUERY:
TABLE CALCULATION_INFO (
  calculation_id autoinc,
  model_info_id int,
  date date,
  time time,
  PRIMARY KEY (calculation_id),
  FOREIGN KEY (model_info_id) REFERENCE model_info(model_id)
); */
static const db_fields_collection calculation_info_fields = {
  db_variable("calculation_id", db_type::type_autoinc,
      {.is_primary_key = true, .is_unique = true, .can_be_null = false}),
  // reference to model_info(fk)
  db_variable("model_info_id", db_type::type_int,
      {.is_reference = true, .can_be_null = false}),
  db_variable("date", db_type::type_date, {.can_be_null = false}),
  db_variable("time", db_type::type_time, {.can_be_null = false})
};
static const db_ref_collection calculation_info_references = {
  db_reference("model_info_id", db_table::table_model_info, "model_id", true)
};
static const db_table_create_setup calculation_info_create_setup(
    db_table::table_calculation_info, calculation_info_fields);

/* SQL_QUERY:
TABLE CALCULATION_STATE_LOG (
  calcaultion_log_id int,
  calcaultion_info_id int,
  volume real,
  pressure real,
  temperature real,
  heat_capac_vol real,
  heat_capac_pres real,
  internal_energy real,
  beta_kr real,
  enthalpy real
  state_phase char(12)
  PRIMARY KEY (calculation_log_id, calculation_info_id),
  FOREIGN KEY (calculation_info_id)
      REFERENCE calculation_info(calculation_id) ON DELETE CASCADE
); */
static const db_fields_collection calculation_state_log_fields = {
  db_variable("calculation_log_id", db_type::type_autoinc,
      {.is_primary_key = true, .is_unique = true, .can_be_null = false}),
  db_variable("calculation_info_id", db_type::type_int,
      {.is_primary_key = true, .is_reference = true, .can_be_null = false}),
  db_variable("volume", db_type::type_real, {}),
  db_variable("pressure", db_type::type_real, {}),
  db_variable("temperature", db_type::type_real, {}),
  db_variable("heat_capacity_vol", db_type::type_real, {}),
  db_variable("heat_capacity_pres", db_type::type_real, {}),
  db_variable("internal_energy", db_type::type_real, {}),
  db_variable("beta_kr", db_type::type_real, {}),
  db_variable("enthalpy", db_type::type_real, {}),
  db_variable("state_phase", db_type::type_char_array,
      {.is_array = true}, 12)
};
static const db_ref_collection calculation_state_log_references = {
  db_reference("calculation_info_id", db_table::table_calculation_info,
      "calculation_id", true, db_reference::db_reference_act::ref_act_cascade,
      db_reference::db_reference_act::ref_act_cascade)
};
static const db_table_create_setup calculation_state_log_create_setup(
    db_table::table_calculation_state_log, calculation_state_log_fields);
}  // namespace table_fields_setup


/* db_variable */
db_variable::db_variable(std::string fname, db_var_type type,
    db_variable_flags flags, int len)
  : fname(fname), type(type), flags(flags), len(len) {}

merror_t db_variable::CheckYourself() const {
  merror_t ew = ERROR_SUCCESS_T;
  if (fname.empty()) {
    ew = ERROR_DB_VARIABLE;
    Logging::Append(io_loglvl::err_logs, "пустое имя поля таблицы бд" +
        STRING_DEBUG_INFO);
  } else if (type == db_var_type::type_char_array &&
      (!flags.is_array || len < 1)) {
    ew = ERROR_DB_VARIABLE;
    Logging::Append(io_loglvl::err_logs,
        "установки поля char array не соответствуют ожиданиям :(" +
        STRING_DEBUG_INFO);
  }
  return ew;
}

namespace ns_tfs = table_fields_setup;

/* db_table_create_setup */
db_table_create_setup::db_table_create_setup(db_table table,
    const db_fields_collection &fields)
  : table(table), fields(fields), ref_strings(nullptr) {
  switch (table) {
    case db_table::table_model_info:
      break;
    case db_table::table_calculation_info:
      ref_strings = &ns_tfs::calculation_info_references;
      break;
    case db_table::table_calculation_state_log:
      ref_strings = &ns_tfs::calculation_state_log_references;
      break;
    default:
      assert(0 && "undef table");
      break;
  }
  setupPrimaryKeyString();
  checkReferences();
  if (init_error.GetErrorCode()) {
    init_error.LogIt();
  }
}

void db_table_create_setup::setupPrimaryKeyString() {
  pk_string.fnames.clear();
  for (const auto &field: fields) {
    if (field.flags.is_primary_key)
      pk_string.fnames.push_back(field.fname);
  }
  assert(pk_string.fnames.empty() == false);
}

// todo: too long function
//   split in few method(for example remove 'switch')
void db_table_create_setup::checkReferences() {
  if (ref_strings) {
    for (auto const &tref : *ref_strings) {
      // check fname present in fields
      bool exist = std::find_if(fields.begin(), fields.end(),
          [&tref](const db_variable &v)
              {return v.fname == tref.fname;}) != fields.end();
      if (exist) {
        const db_fields_collection *ffields= nullptr;
        switch (tref.foreign_table) {
          case db_table::table_model_info:
            ffields = &ns_tfs::model_info_fields;
            break;
          case db_table::table_calculation_info:
            ffields = &ns_tfs::calculation_info_fields;
            break;
          case db_table::table_calculation_state_log:
            ffields = &ns_tfs::calculation_state_log_fields;
            break;
          default:
            assert(0 && "undef table");
            break;
        }
        exist = std::find_if(ffields->begin(), ffields->end(),
            [&tref](const db_variable &v)
                {return v.fname == tref.foreign_fname;}) != ffields->end();
        if (!exist) {
          init_error.SetError(ERROR_DB_REFER_FIELD,
              "Неверное имя внешнего поля для reference.\n"
              "Таблица - " + get_table_name(table) + "\n"
              "Внешняя таблица - " + get_table_name(tref.foreign_table) + "\n"
              + STRING_DEBUG_INFO);
          break;
        }
      } else {
        init_error.SetError(ERROR_DB_REFER_FIELD,
            "Неверное собственное имя поля для reference" + STRING_DEBUG_INFO);
        break;
      }
    }
  }
}

db_reference::db_reference(const std::string &fname, db_table ftable,
    const std::string &f_fname, bool is_fkey, db_reference_act on_del,
    db_reference_act on_upd)
  : fname(fname), foreign_table(ftable), foreign_fname(f_fname),
    is_foreign_key(is_fkey), has_on_delete(false), delete_method(on_del),
    update_method(on_upd) {
  if (delete_method != db_reference_act::ref_act_empty)
    has_on_delete = true;
  if (update_method != db_reference_act::ref_act_empty)
    has_on_update = true;
}

merror_t db_reference::CheckYourself() const {
  merror_t error = ERROR_SUCCESS_T;
  if (!fname.empty()) {
    if (!foreign_fname.empty()) {
      // перестраховались
      if (has_on_delete && delete_method == db_reference_act::ref_act_empty) {
        error = ERROR_DB_VARIABLE;
        Logging::Append(io_loglvl::err_logs, "несоответсвие метода удаления "
            "для ссылки. Поле: " + fname + ". Внешнее поле: " + foreign_fname);
      }
      if (has_on_update && update_method == db_reference_act::ref_act_empty) {
        error = ERROR_DB_VARIABLE;
        Logging::Append(io_loglvl::err_logs, "несоответсвие метода обновления "
            "для ссылки. Поле: " + fname + ". Внешнее поле: " + foreign_fname);
      }
    } else {
      error = ERROR_DB_VARIABLE;
      Logging::Append(io_loglvl::err_logs, "пустое имя поля для "
          "внешней таблицы бд\n");
    }
  } else {
    error = ERROR_DB_VARIABLE;
      Logging::Append(io_loglvl::err_logs, "пустое имя поля таблицы бд\n");
  }
  return error;
}

/* this is for postgresql, what about anothers? */
std::string db_reference::GetReferenceActString(db_reference_act act) {
  switch (act) {
    case db_reference_act::ref_act_empty:
      return "";
    case db_reference_act::ref_act_cascade:
      return "CASCADE";
    case db_reference_act::ref_act_restrict:
      return "RESTRICT";
  }
  return "";
}

/* last update 2020.03.15: добавились флаги моделей, они
 *   учитываются в версии модели */
// static_assert(sizeof(model_info) == 104, "Необходимо перепроверить "
//     "функцию table_model_info() - вероятно изменился формат струтуры данных "
//     "model_info добавьте новые поля, или измените старые");
/** \brief функция собирающая набор полей для
  *   создания таблицы БД model_info информации о модели */
static const db_table_create_setup &table_create_model_info() {
  return ns_tfs::model_info_create_setup;
}

// static_assert(sizeof(calculation_info) == 24, "См static_assert "
//     "для table_cteate_model_info, идея таже");
/** \brief функция собирающая набор полей для
  *   создания таблицы БД calculation_info информации о расчёте */
static const db_table_create_setup &table_create_calculation_info() {
  return ns_tfs::calculation_info_create_setup;
}

// static_assert(sizeof(calculation_state_log) == 112, "См static_assert "
//     "для table_cteate_model_info, идея таже");
/** \brief функция собирающая набор полей для
  *   создания таблицы БД calculation_state_log строку расчёта */
static const db_table_create_setup &table_create_calculation_state_log() {
  return ns_tfs::calculation_state_log_create_setup;
}

/* todo: maybe replace with pointer */
const db_table_create_setup &get_table_create_setup(db_table dt) {
  switch (dt) {
    case db_table::table_model_info:
      return table_create_model_info();
    case db_table::table_calculation_info:
      return table_create_calculation_info();
    case db_table::table_calculation_state_log:
      return table_create_calculation_state_log();
  }
  assert(0 && "undef table");
}

namespace ns_ucf = update_configuration_functional;

/* db_parameters */
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
  // дефайн на гугло десты БД
#if !defined(DATABASE_TEST)
  is_dry_run_ = ProgramState::Instance().IsDryRunDBConn();
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
