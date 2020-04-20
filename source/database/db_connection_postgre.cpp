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

#include "common.h"
#include "db_query.h"
#include "file_structs.h"
#include "models_configurations.h"
#include "program_state.h"
#include "Logging.h"

#include <map>
#include <sstream>
#ifdef _DEBUG
#  include <iostream>
#endif  // !_DEBUG

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

void DBConnectionPostgre::Commit() {}

void DBConnectionPostgre::Rollback() {}

mstatus_t DBConnectionPostgre::CreateTable(
    const db_table_create_setup &fields) {
  std::stringstream create_ss = setupCreateTableString(fields);
  if (is_connected_ && pconnect_) {
    if (pconnect_->is_open() && !error_.GetErrorCode()) {
      try {
        pqxx::work tr(*pconnect_);
        tr.exec(create_ss.str());
        tr.commit();
        if (IS_DEBUG_MODE)
          Logging::Append(io_loglvl::debug_logs, "Запрос БД на создание таблицы:"
              + create_ss.str() + "\n\t");
      } catch (const pqxx::integrity_constraint_violation &e) {
        status_ = STATUS_HAVE_ERROR;
        error_.SetError(ERROR_DB_SQL_QUERY, "Exception text: " +
            std::string(e.what()) + "\n Query: " + e.query());
      } catch (const std::exception &e) {
        error_.SetError(ERROR_DB_CONNECTION,
            "Подключение к БД: exception. Запрос:\n" + create_ss.str()
            + "\nexception what: " + e.what());
        status_ = STATUS_HAVE_ERROR;
      }
    } else {
      // is not connected
      if (!error_.GetErrorCode()) {
        error_.SetError(ERROR_DB_CONNECTION, "Соединение с бд не открыто");
        status_ = STATUS_NOT;
      }
    }
  } else {
    if (is_dry_run_) {
      // dry_run_ programm setup
      Logging::Append(io_loglvl::debug_logs, "dry_run: " + create_ss.str());
    } else {
      // error - not connected
      error_.SetError(ERROR_DB_CONNECTION);
      status_ = STATUS_NOT;
    }
  }
  return status_;
}

void DBConnectionPostgre::UpdateTable(db_table t,
    const db_table_select_setup &vals) {
  (void) t;
  (void) vals;
  assert(0);
}

void DBConnectionPostgre::InsertModelInfo(const model_info &mi) {
  (void) mi;
  assert(0);
}

void DBConnectionPostgre::SelectModelInfo(
    const db_table_select_setup &mip) {
  (void) mip;
  assert(0);
}

void DBConnectionPostgre::InsertCalculationInfo(
    const calculation_info &ci) {
  (void) ci;
  assert(0);
}

void DBConnectionPostgre::InsertCalculationStateLog(
    const calculation_state_log &sl) {
  (void) sl;
  assert(0);
}

DBConnectionPostgre::~DBConnectionPostgre() {
  CloseConnection();
}

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

mstatus_t DBConnectionPostgre::IsTableExists(db_table t, bool *is_exists) {
  std::stringstream select_ss;
  select_ss << "SELECT EXISTS ( SELECT 1 FROM information_schema.tables "
      "WHERE table_schema = 'public' AND table_name = '" <<
      get_table_name(t) << "');";
  if (is_connected_ && pconnect_) {
    if (pconnect_->is_open()) {
      try {
        pqxx::work tr(*pconnect_);
        pqxx::result trres(tr.exec(select_ss.str()));
        // в примерах коммитят после запроса
        tr.commit();

        if (!trres.empty()) {
          std::string ex = trres.begin()[0].as<std::string>();
          *is_exists = (ex == "t") ? true : false;
          // неплохо бы залогировать результат
          // for example:
          //   std::cout << qres.begin()[0].as<std::string>() << std::endl;
          if (IS_DEBUG_MODE)
            Logging::Append(io_loglvl::debug_logs, "Ответ на запрос БД:"
                + select_ss.str() + "\n\t" + trres.begin()[0].as<std::string>());
        }
      } catch (const pqxx::undefined_table &) {
        *is_exists = false;
      } catch (const std::exception &e) {
        error_.SetError(ERROR_DB_CONNECTION,
            "Подключение к БД: exception. Запрос:\n" + select_ss.str()
            + "\nexception what: " + e.what() + "\n" + STRING_DEBUG_INFO);
        status_ = STATUS_HAVE_ERROR;
      }
    }
  }
  if (is_dry_run_)
    Logging::Append(io_loglvl::debug_logs, "dry_run: " + select_ss.str());
  return status_;
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

std::stringstream DBConnectionPostgre::setupCreateTableString(
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
    // REFERENCE
    if (fields.ref_strings) {
      for (const auto &ref : *fields.ref_strings) {
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

/* можно ещё разнести:
 *   вообще вынести эту функцию в родительский класс */
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

std::string DBConnectionPostgre::db_reference_to_string(
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

std::string DBConnectionPostgre::db_primarykey_to_string(
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
