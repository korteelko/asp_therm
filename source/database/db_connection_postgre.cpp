/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#include "db_connection_postgre.h"

#include "db_queries_setup.h"
#include "db_query.h"
#include "file_structs.h"
#include "models_configurations.h"

#include <algorithm>
#include <map>
#include <numeric>
#include <sstream>
#include <vector>

#include <assert.h>


/* примеры использования можно посмотреть на github, в репке Jeroen Vermeulen:
     https://github.com/jtv/libpqxx */
namespace postgresql_impl {
static std::map<db_type, std::string> str_db_types =
    std::map<db_type, std::string>{
  {db_type::type_autoinc, "SERIAL"},
  {db_type::type_uuid, "UUID"},
  {db_type::type_bool, "BOOL"},
  {db_type::type_int, "INTEGER"},
  {db_type::type_long, "BIGINT"},
  {db_type::type_real, "REAL"},
  {db_type::type_date, "DATE"},
  {db_type::type_time, "TIME"},
  {db_type::type_char_array, "CHAR"},
  {db_type::type_text, "TEXT"},
};

struct where_string_set {
  where_string_set(pqxx::nontransaction *tr)
    : tr(tr) {}
  std::string operator()(db_type t, const std::string &f,
      const std::string &v) {
    if (t == db_type::type_date) {
      return f + " = " + DBConnectionPostgre::DateToPostgreDate(v);
    } else if (t == db_type::type_time) {
      return f + " = " + DBConnectionPostgre::TimeToPostgreTime(v);
    }
    return (t == db_type::type_char_array || t == db_type::type_text) ?
        f + " = " + tr->quote(v): f + " = " + v;
  }
public:
  pqxx::nontransaction *tr;
};
}  // namespace postgresql_impl

DBConnectionPostgre::DBConnectionPostgre(const db_parameters &parameters)
  : DBConnection(parameters) {}

DBConnectionPostgre::~DBConnectionPostgre() {
  CloseConnection();
}

mstatus_t DBConnectionPostgre::AddSavePoint(const db_save_point &sp) {
  return exec_wrap<db_save_point, void,
     std::stringstream (DBConnectionPostgre::*)(const db_save_point &),
     void (DBConnectionPostgre::*)(const std::stringstream &, void *)>(
         sp, nullptr, &DBConnectionPostgre::setupAddSavePointString,
         &DBConnectionPostgre::execAddSavePoint);
}

void DBConnectionPostgre::RollbackToSavePoint(const db_save_point &sp) {
  exec_wrap<db_save_point, void,
     std::stringstream (DBConnectionPostgre::*)(const db_save_point &),
     void (DBConnectionPostgre::*)(const std::stringstream &, void *)>(
         sp, nullptr, &DBConnectionPostgre::setupRollbackToSavePoint,
         &DBConnectionPostgre::execRollbackToSavePoint);
}

mstatus_t DBConnectionPostgre::SetupConnection() {
  auto connect_str = setupConnectionString();
  if (!is_dry_run_ ) {
    try {
      pqxx_work.InitConnection(connect_str);
      if (!error_.GetErrorCode()) {
        if (pqxx_work.IsAvailable()) {
          status_ = STATUS_OK;
          is_connected_ = true;
          // отметим начало транзакции
          // todo: вероятно в отдельную функцию вынести
          pqxx_work.GetTransaction()->exec("begin;");
          if (IS_DEBUG_MODE)
            Logging::Append(io_loglvl::debug_logs, "Подключение к БД "
                + parameters_.name);
        } else {
          error_.SetError(ERROR_DB_CONNECTION,
              "Подключение к БД не открыто. Запрос:\n" + connect_str);
          status_ = STATUS_HAVE_ERROR;
        }
      } else {
        error_.SetError(ERROR_DB_CONNECTION,
            "Подключение к БД: инициализация. Запрос:\n" + connect_str);
        status_ = STATUS_HAVE_ERROR;
      }
    } catch(const std::exception &e) {
      error_.SetError(ERROR_DB_CONNECTION,
          "Подключение к БД: exception. Запрос:\n" + connect_str
          + "\nexception what: " + e.what());
      status_ = STATUS_HAVE_ERROR;
    }
  } else {
    status_ = STATUS_OK;
    Logging::Append(io_loglvl::debug_logs, "dry_run connect:" + connect_str);
    Logging::Append(io_loglvl::debug_logs, "dry_run transaction begin");
  }
  return status_;
}

void DBConnectionPostgre::CloseConnection() {
  if (pqxx_work.pconnect_) {
    // если собирали транзакцию - закрыть
    if (pqxx_work.IsAvailable())
      pqxx_work.GetTransaction()->exec("commit;");
    pqxx_work.pconnect_->disconnect();
    is_connected_ = false;
    error_.Reset();
    if (IS_DEBUG_MODE)
      Logging::Append(io_loglvl::debug_logs, "Закрытие соединения c БД "
          + parameters_.name);
  }
  if (is_dry_run_) {
    status_ = STATUS_OK;
    Logging::Append(io_loglvl::debug_logs, "dry_run commit and disconect");
  }
}

mstatus_t DBConnectionPostgre::IsTableExists(db_table t, bool *is_exists) {
  return exec_wrap<db_table, bool,
     std::stringstream (DBConnectionPostgre::*)(db_table),
     void (DBConnectionPostgre::*)(const std::stringstream &, bool *)>(
         t, is_exists, &DBConnectionPostgre::setupTableExistsString,
         &DBConnectionPostgre::execIsTableExists);
}

mstatus_t DBConnectionPostgre::CheckTableFormat(
    const db_table_create_setup &fields) {
  // такс, проверяем:
  //   колонки
  std::vector<std::string> not_exists_cols;
  mstatus_t res = exec_wrap<db_table, std::vector<std::string>,
     std::stringstream (DBConnectionPostgre::*)(db_table),
     void (DBConnectionPostgre::*)(const std::stringstream &, std::vector<std::string> *)>(
         fields.table, &not_exists_cols, &DBConnectionPostgre::setupColumnNamesString,
         &DBConnectionPostgre::execColumnNamesString);
  if (is_status_ok(res)) {
    for (const db_variable &fieldname : fields.fields) {
      if (std::find(not_exists_cols.begin(), not_exists_cols.end(),
          fieldname.fname) == not_exists_cols.end()) {
        error_.SetError(ERROR_DB_COL_EXISTS,
            "В таблице не существует колонка " + fieldname.fname);
        break;
      }
    }
  } else {
    return res;
  }
  // todo:
  //   примари ключ

  //   внешние ключи

  //   уникальные наборы

  return res;
}

mstatus_t DBConnectionPostgre::UpdateTable(const db_table_create_setup &fields) {
  std::vector<std::string> not_exists_cols;
  /* append not exists columns */
  mstatus_t res = exec_wrap<db_table, std::vector<std::string>,
     std::stringstream (DBConnectionPostgre::*)(db_table),
     void (DBConnectionPostgre::*)(const std::stringstream &, std::vector<std::string> *)>(
         fields.table, &not_exists_cols, &DBConnectionPostgre::setupColumnNamesString,
         &DBConnectionPostgre::execColumnNamesString);
  if (is_status_ok(res)) {
    // добавить строки
    for (const db_variable &field: fields.fields) {
      if (std::find(not_exists_cols.begin(), not_exists_cols.end(),
          field.fname) == not_exists_cols.end()) {
        // add column
        std::pair<db_table, const db_variable &> pdv{fields.table, field};
        res = exec_wrap<const std::pair<db_table, const db_variable &>, void,
            std::stringstream (DBConnectionPostgre::*)(const std::pair<db_table,
                const db_variable &> &),
            void (DBConnectionPostgre::*)(const std::stringstream &, void *)>(
                pdv, nullptr, &DBConnectionPostgre::setupAddColumnString,
                &DBConnectionPostgre::execAddColumn);
      }
    }
  } else {
    return res;
  }
  return STATUS_HAVE_ERROR;
}

mstatus_t DBConnectionPostgre::CreateTable(
    const db_table_create_setup &fields) {
  return exec_wrap<db_table_create_setup, void,
     std::stringstream (DBConnectionPostgre::*)(const db_table_create_setup &),
     void (DBConnectionPostgre::*)(const std::stringstream &, void *)>(
         fields, nullptr, &DBConnectionPostgre::setupCreateTableString,
         &DBConnectionPostgre::execCreateTable);
}

mstatus_t DBConnectionPostgre::DropTable(const db_table_drop_setup &drop) {
  return exec_wrap<db_table_drop_setup, void,
     std::stringstream (DBConnectionPostgre::*)(const db_table_drop_setup &),
     void (DBConnectionPostgre::*)(const std::stringstream &, void *)>(
         drop, nullptr, &DBConnectionPostgre::setupDropTableString,
         &DBConnectionPostgre::execDropTable);
}

mstatus_t DBConnectionPostgre::InsertRows(
    const db_query_insert_setup &insert_data) {
  return exec_wrap<db_query_insert_setup, void,
     std::stringstream (DBConnectionPostgre::*)(const db_query_insert_setup &),
     void (DBConnectionPostgre::*)(const std::stringstream &, void *)>(
         insert_data, nullptr, &DBConnectionPostgre::setupInsertString,
         &DBConnectionPostgre::execInsert);
}

mstatus_t DBConnectionPostgre::DeleteRows(
    const db_query_delete_setup &delete_data) {
  return exec_wrap<db_query_delete_setup, void,
     std::stringstream (DBConnectionPostgre::*)(const db_query_delete_setup &),
     void (DBConnectionPostgre::*)(const std::stringstream &, void *)>(
         delete_data, nullptr, &DBConnectionPostgre::setupDeleteString,
         &DBConnectionPostgre::execDelete);
}

mstatus_t DBConnectionPostgre::SelectRows(
    const db_query_select_setup &select_data,
    db_query_select_result *result_data) {
  pqxx::result result;
  result_data->values_vec.clear();
  mstatus_t res = exec_wrap<db_query_select_setup, pqxx::result,
     std::stringstream (DBConnectionPostgre::*)(const db_query_select_setup &),
     void (DBConnectionPostgre::*)(const std::stringstream &, pqxx::result *)>(
         select_data, &result, &DBConnectionPostgre::setupSelectString,
         &DBConnectionPostgre::execSelect);
  for (pqxx::const_result_iterator::reference row: result) {
    db_query_basesetup::row_values rval;
    db_query_basesetup::field_index ind = 0;
    for (const auto &field: select_data.fields) {
      std::string fieldname = field.fname;
      if (row[fieldname] != row.end())
        rval.emplace(ind, row[fieldname].c_str());
      ++ind;
    }
    result_data->values_vec.push_back(rval);
  }
  return res;
}

mstatus_t DBConnectionPostgre::UpdateRows(
    const db_query_update_setup &update_data) {
  return exec_wrap<db_query_update_setup, void,
     std::stringstream (DBConnectionPostgre::*)(const db_query_update_setup &),
     void (DBConnectionPostgre::*)(const std::stringstream &, void *)>(
         update_data, nullptr, &DBConnectionPostgre::setupUpdateString,
         &DBConnectionPostgre::execUpdate);
}


std::string DBConnectionPostgre::setupConnectionString() {
  std::stringstream connect_ss;
  connect_ss << "dbname = " << parameters_.name << " ";
  connect_ss << "user = " << parameters_.username << " ";
  connect_ss << "password = " << parameters_.password << " ";
  connect_ss << "hostaddr = " << parameters_.host << " ";
  connect_ss << "port = " << parameters_.port;
  return connect_ss.str();
}

std::stringstream DBConnectionPostgre::setupTableExistsString(db_table t) {
  std::stringstream select_ss;
  select_ss << "SELECT EXISTS ( SELECT 1 FROM information_schema.tables "
      "WHERE table_schema = 'public' AND table_name = '" <<
      get_table_name(t) << "');";
  return select_ss;
}
std::stringstream DBConnectionPostgre::setupColumnNamesString(db_table t) {
  std::stringstream select_ss;
  select_ss << "SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS "
      "WHERE TABLE_NAME = '" << get_table_name(t) << "';";
  return select_ss;
}
std::stringstream DBConnectionPostgre::setupInsertString(
    const db_query_insert_setup &fields) {
  if (fields.values_vec.empty()) {
    error_.SetError(ERROR_DB_VARIABLE, "Нет данных для INSERT операции");
    return std::stringstream();
  }
  std::string fnames = "INSERT INTO " + get_table_name(fields.table) + " (";
  // std::string values = "VALUES (";
  std::vector<std::string> vals;
  std::vector<std::string> rows(fields.values_vec.size());
  db_variable::db_var_type t;
  auto txn = pqxx_work.GetTransaction();
  // set fields:
  auto &row = fields.values_vec[0];
  for (auto &x: row)
    fnames += fields.fields[x.first].fname + ", ";
  fnames.replace(fnames.size() - 2, fnames.size() - 1, ")");

  for (const auto &row: fields.values_vec) {
    std::string value = " (";
    for (const auto &x : row) {
      if (x.first >= 0 && x.first < fields.fields.size()) {
        t = fields.fields[x.first].type;
        if (txn && (t == db_variable::db_var_type::type_char_array ||
            t == db_variable::db_var_type::type_text)) {
          /* хз, но в интернетах так */
          value += txn->quote(x.second) + ", ";
        } else if (t == db_variable::db_var_type::type_date) {
          value += DateToPostgreDate(x.second) + ", ";
        } else if (t == db_variable::db_var_type::type_time) {
          value += TimeToPostgreTime(x.second) + ", ";
        } else {
          value += x.second + ", ";
        }
      } else {
        Logging::Append(io_loglvl::debug_logs, "Ошибка индекса операции INSERT.\n"
            "\tДля таблицы " + get_table_name(fields.table));
      }
    }
    value.replace(value.size() - 2, value.size(), "),");
    vals.emplace_back(value);
  }
  vals.back().replace(vals.back().size() - 1, vals.back().size(), ";");
  std::stringstream sstr;
  sstr << fnames << " VALUES ";
  for (const auto &x: vals)
    sstr << x;
  return sstr;
}

std::stringstream DBConnectionPostgre::setupDeleteString(
    const db_query_delete_setup &fields) {
  std::stringstream sstr;
  postgresql_impl::where_string_set ws(pqxx_work.GetTransaction());
  sstr << "DELETE FROM " << get_table_name(fields.table);
  if (fields.where_condition != nullptr)
    sstr << " WHERE " << fields.where_condition->GetString(ws);
  sstr << ";";
  return sstr;
}
std::stringstream DBConnectionPostgre::setupSelectString(
    const db_query_select_setup &fields) {
  std::stringstream sstr;
  postgresql_impl::where_string_set ws(pqxx_work.GetTransaction());
  sstr << "SELECT * FROM " << get_table_name(fields.table);
  if (fields.where_condition != nullptr)
    sstr << " WHERE " << fields.where_condition->GetString(ws);
  sstr << ";";
  return sstr;
}
std::stringstream DBConnectionPostgre::setupUpdateString(
    const db_query_update_setup &fields) {
  postgresql_impl::where_string_set ws(pqxx_work.GetTransaction());
  std::stringstream sstr;
  if (!fields.values.empty()) {
    sstr << "UPDATE " << get_table_name(fields.table)
        << " SET ";
    std::string set_str = "";
    for (const auto &x: fields.values)
      set_str += fields.fields[x.first].fname + " = " + x.second + ",";
    set_str[set_str.size() - 1] = ' ';
    sstr << set_str;
    if (fields.where_condition != nullptr)
      sstr << " WHERE " << fields.where_condition->GetString(ws);
    sstr << ";";
  }
  return sstr;
}

void DBConnectionPostgre::execNoReturn(const std::stringstream &sstr) {
  auto tr = pqxx_work.GetTransaction();
  if (tr)
    tr->exec0(sstr.str());
}
void DBConnectionPostgre::execAddSavePoint(
    const std::stringstream &sstr, void *) {
  execNoReturn(sstr);
}
void DBConnectionPostgre::execRollbackToSavePoint(
    const std::stringstream &sstr, void *) {
  execNoReturn(sstr);
}
void DBConnectionPostgre::execIsTableExists(
    const std::stringstream &sstr, bool *is_exists) {
  auto tr = pqxx_work.GetTransaction();
  if (tr) {
    pqxx::result trres(tr->exec(sstr.str()));
    if (!trres.empty()) {
      std::string ex = trres.begin()[0].as<std::string>();
      *is_exists = (ex == "t") ? true : false;
      if (IS_DEBUG_MODE)
       Logging::Append(io_loglvl::debug_logs, "Ответ на запрос БД:"
            + sstr.str() + "\t'" + trres.begin()[0].as<std::string>() + "'\n") ;
    }
  }
}
void DBConnectionPostgre::execColumnNamesString(
    const std::stringstream &sstr, std::vector<std::string> *column_names) {
  column_names->clear();
  auto tr = pqxx_work.GetTransaction();
  if (tr) {
    pqxx::result trres(tr->exec(sstr.str()));
    if (!trres.empty()) {
      std::transform(trres.begin(), trres.end(), column_names->end(),
          [](pqxx::const_result_iterator::reference x)
              { return trim_str(x[0].as<std::string>()); });
      if (IS_DEBUG_MODE) {
        std::string cols = std::accumulate(
            column_names->begin(), column_names->end(), std::string(),
            [](std::string &r, const std::string &n) { return r + n; });
        Logging::Append(io_loglvl::debug_logs, "Ответ на запрос БД:"
            + sstr.str() + "\t'" + cols + "'\n") ;
      }
    }
  }
}
void DBConnectionPostgre::execAddColumn(const std::stringstream &sstr, void *) {
  execNoReturn(sstr);
}
void DBConnectionPostgre::execCreateTable(const std::stringstream &sstr, void *) {
  execNoReturn(sstr);
}
void DBConnectionPostgre::execDropTable(const std::stringstream &sstr, void *) {
  execNoReturn(sstr);
}
void DBConnectionPostgre::execInsert(const std::stringstream &sstr, void *) {
  execNoReturn(sstr);
}
void DBConnectionPostgre::execDelete(const std::stringstream &sstr, void *) {
  execNoReturn(sstr);
}
void DBConnectionPostgre::execSelect(const std::stringstream &sstr,
    pqxx::result *result) {
  auto tr = pqxx_work.GetTransaction();
  if (tr)
    *result = tr->exec(sstr.str());
}
void DBConnectionPostgre::execUpdate(const std::stringstream &sstr, void *) {
  execNoReturn(sstr);
}

std::string DBConnectionPostgre::db_variable_to_string(
    const db_variable &dv) {
  std::stringstream ss;
  merror_t ew = dv.CheckYourself();
  if (!ew) {
    ss << dv.fname << " ";
    auto itDBtype = postgresql_impl::str_db_types.find(dv.type);
    if (itDBtype != postgresql_impl::str_db_types.end()) {
      ss << postgresql_impl::str_db_types[dv.type];
      if (dv.flags.is_array)
        ss << "(" << dv.len << ")";
      if (!dv.flags.can_be_null)
        ss << " NOT NULL";
      if (dv.flags.has_default)
        ss << " DEFAULT " << dv.default_str;
    } else {
      error_.SetError(ERROR_DB_VARIABLE,
          "Тип переменной не задан для данной имплементации БД");
    }
  } else {
    error_.SetError(ew,
        "Проверка параметров поля таблицы завершилось ошибкой");
  }
  return ss.str();
}

std::string DBConnectionPostgre::DateToPostgreDate(const std::string &date) {
  // postgres need
  char s[16] = {0};
  int i = 0;
  for_each (date.begin(), date.end(),
      [&s, &i](char c) { s[i++] = (c == '/') ? '-' : c; });
  return "'" + std::string(s) + "'";
}

std::string DBConnectionPostgre::TimeToPostgreTime(const std::string &time) {
  return "'" + time + "'";
}

std::string DBConnectionPostgre::PostgreDateToDate(const std::string &pdate) {
  char s[16] = {0};
  int i = 0;
  for_each (pdate.begin(), pdate.end(),
      [&s, &i](char c) { s[i++] = (c == '-') ? '/' : c; });
  return s;
}

std::string DBConnectionPostgre::PostgreTimeToTime(const std::string &ptime) {
  return ptime;
}
