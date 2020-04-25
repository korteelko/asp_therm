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
#include <sstream>

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
}  // namespace postgresql_impl

DBConnectionPostgre::DBConnectionPostgre(const db_parameters &parameters)
  : DBConnection(parameters), pconnect_(nullptr) {}

DBConnectionPostgre::~DBConnectionPostgre() {
  CloseConnection();
}

void DBConnectionPostgre::Commit() {}

void DBConnectionPostgre::Rollback() {}

mstatus_t DBConnectionPostgre::SetupConnection() {
  auto connect_str = setupConnectionString();
  if (!is_dry_run_ ) {
    try {
      pconnect_ = std::unique_ptr<pqxx::connection>(
          new pqxx::connection(connect_str));
      if (pconnect_ && !error_.GetErrorCode()) {
        if (pconnect_->is_open()) {
          status_ = STATUS_OK;
          is_connected_ = true;
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
    Logging::Append(io_loglvl::debug_logs, "dry_run connect:" + connect_str);
  }
  return status_;
}

void DBConnectionPostgre::CloseConnection() {
  if (pconnect_) {
    pconnect_->disconnect();
    is_connected_ = false;
    if (IS_DEBUG_MODE)
      Logging::Append(io_loglvl::debug_logs, "Закрытие соединения c БД "
          + parameters_.name);
  }
  if (is_dry_run_) {
    Logging::Append(io_loglvl::debug_logs, "dry_run disconect");
  }
}

mstatus_t DBConnectionPostgre::CreateTable(
    const db_table_create_setup &fields) {
  return exec_op<db_table_create_setup, void,
     std::stringstream (DBConnectionPostgre::*)(const db_table_create_setup &),
     void (DBConnectionPostgre::*)(const std::stringstream &, void *)>(
         fields, nullptr, &DBConnectionPostgre::setupCreateTableString,
         &DBConnectionPostgre::execCreateTable);
}

void DBConnectionPostgre::UpdateTable(db_table t,
    const db_table_update_setup &vals) {
  (void) t;
  (void) vals;
  assert(0);
}

mstatus_t DBConnectionPostgre::IsTableExists(db_table t, bool *is_exists) {
  return exec_op<db_table, bool,
     std::stringstream (DBConnectionPostgre::*)(db_table),
     void (DBConnectionPostgre::*)(const std::stringstream &, bool *)>(
         t, is_exists, &DBConnectionPostgre::setupTableExistsString,
         &DBConnectionPostgre::execIsTableExists);
}

mstatus_t DBConnectionPostgre::InsertRows(
    const db_table_insert_setup &insert_data) {
  return exec_op<db_table_insert_setup, void,
     std::stringstream (DBConnectionPostgre::*)(const db_table_insert_setup &),
     void (DBConnectionPostgre::*)(const std::stringstream &, void *)>(
         insert_data, nullptr, &DBConnectionPostgre::setupInsertString,
         &DBConnectionPostgre::execInsert);
}

mstatus_t DBConnectionPostgre::DeleteRows(
    const db_table_delete_setup &delete_data) {
  return exec_op<db_table_delete_setup, void,
     std::stringstream (DBConnectionPostgre::*)(const db_table_delete_setup &),
     void (DBConnectionPostgre::*)(const std::stringstream &, void *)>(
         delete_data, nullptr, &DBConnectionPostgre::setupDeleteString,
         &DBConnectionPostgre::execDelete);
}

mstatus_t DBConnectionPostgre::SelectRows(
    const db_table_select_setup &select_data) {
  assert(0);
}

mstatus_t DBConnectionPostgre::UpdateRows(
    const db_table_update_setup &update_data) {
  assert(0);
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
std::stringstream DBConnectionPostgre::setupInsertString(
    const db_table_insert_setup &fields) {
  std::string fnames;
  if (fields.values_vec.empty()) {
    error_.SetError(ERROR_DB_VARIABLE, "Нет данных для INSERT операции");
    return std::stringstream();
  }
  fnames = "INSERT INTO " + get_table_name(fields.table) + " (";
  std::string values = "VALUES (";
  std::vector<std::string> rows(fields.values_vec.size());
  db_variable::db_var_type t;
  std::unique_ptr<pqxx::work> txn;
  if (pconnect_)
    txn.reset(new pqxx::work(*pconnect_));
  for (const auto &row: fields.values_vec) {
    for (const auto &x : row) {
      if (x.first >= 0 && x.first < fields.fields.size()) {
        fnames += fields.fields[x.first].fname + ", ";
        t = fields.fields[x.first].type;
        if (txn && (t == db_variable::db_var_type::type_char_array ||
            t == db_variable::db_var_type::type_text)) {
          /* хз, но в интернетах так */
          values += txn->quote(x.second) + ", ";
        } else {
          values += x.second + ", ";
        }
      } else {
        Logging::Append(io_loglvl::debug_logs, "Ошибка индекса операции INSERT.\n"
            "\tДля таблицы " + get_table_name(fields.table));
      }
    }
  }
  fnames.replace(fnames.size() - 2, fnames.size() - 1, ")");
  values.replace(values.size() - 2, values.size() - 1, ")");
  std::stringstream sstr;
  sstr << fnames << values << ";";
  return sstr;
}

void DBConnectionPostgre::execIsTableExists(
    const std::stringstream &sstr, bool *is_exists) {
  pqxx::work tr(*pconnect_);
  pqxx::result trres(tr.exec(sstr.str()));
  if (!trres.empty()) {
    std::string ex = trres.begin()[0].as<std::string>();
    *is_exists = (ex == "t") ? true : false;
    if (IS_DEBUG_MODE)
      Logging::Append(io_loglvl::debug_logs, "Ответ на запрос БД:"
          + sstr.str() + "\t'" + trres.begin()[0].as<std::string>() +
          "'\n") ;
  }
}
void DBConnectionPostgre::execCreateTable(const std::stringstream &sstr, void *) {
  pqxx::work tr(*pconnect_);
  tr.exec0(sstr.str());
  tr.commit();
}
void DBConnectionPostgre::execInsert(const std::stringstream &sstr, void *) {
  pqxx::work tr(*pconnect_);
  tr.exec0(sstr.str());
  tr.commit();
}
void DBConnectionPostgre::execDelete(const std::stringstream &sstr, void *) {
  pqxx::work tr(*pconnect_);
  tr.exec0(sstr.str());
  tr.commit();
}
void DBConnectionPostgre::execSelect(const std::stringstream &sstr,
    pqxx::result *result) {
  pqxx::work tr(*pconnect_);
  *result = tr.exec(sstr.str());
  tr.commit();
}
void DBConnectionPostgre::execUpdate(const std::stringstream &sstr, void *) {
  assert(0);
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
    } else {
      error_.SetError(ERROR_DB_VARIABLE,
          "Тип переменной не задан для данной имплементации БД");
    }
    if (dv.flags.is_array)
      ss << "(" << dv.len << ")";
    if (!dv.flags.can_be_null)
      ss << " NOT NULL";
    if (dv.flags.has_default)
      ss << " DEFAULT " << dv.default_str;
  } else {
    error_.SetError(ew,
        "Проверка параметров поля таблицы завершилось ошибкой");
  }
  return ss.str();
}

std::string DBConnectionPostgre::dateToPostgreDate(const std::string &date) {
  // postgres need
  char s[16] = {0};
  int i = 0;
  for_each (date.begin(), date.end(),
      [&s, &i](char c) { s[i++] = (c == '/') ? '-' : c; });
  return s;
}

std::string DBConnectionPostgre::timeToPostgreTime(const std::string &time) {
  // format postgresql is same as 'hh:mm:ss'
  return time;
}

std::string DBConnectionPostgre::postgreDateToDate(const std::string &pdate) {
  char s[16] = {0};
  int i = 0;
  for_each (pdate.begin(), pdate.end(),
      [&s, &i](char c) { s[i++] = (c == '-') ? '/' : c; });
  return s;
}

std::string DBConnectionPostgre::postgreTimeToTime(const std::string &ptime) {
  return ptime;
}
