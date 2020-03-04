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
#include "Logging.h"

#include <pqxx/pqxx>

#include <map>
#include <sstream>

#include <assert.h>

/* примеры использования можно посмотреть на github, в репке Jeroen Vermeulen:
     https://github.com/jtv/libpqxx */


namespace postgresql_impl {
static std::map<db_type, std::string> str_db_types =
    std::map<db_type, std::string>{
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

void DBConnectionPostgre::Commit() {
  assert(0);
}

void DBConnectionPostgre::Rollback() {
  assert(0);
}

mstatus_t DBConnectionPostgre::CreateTable(
    const db_table_create_setup &fields) {
  if (is_connected_ && pconnect_) {
    std::stringstream create_ss = setupCreateTableString(fields);
    if (status_ != STATUS_DRY_RUN && pconnect_->is_open()) {
      try {
        assert(0 && "otsuda");
        pqxx::work tr(*pconnect_);
        pqxx::result trres(tr.exec(create_ss.str()));
        // в примерах коммитят после запроса
        tr.commit();

        if (!trres.empty()) {
          if (ProgramState::Instance().IsDebugMode())
            Logging::Append(io_loglvl::debug_logs, "Запрос БД на создание таблицы:"
                + create_ss.str() + "\n\t");
        }
      } catch (const pqxx::undefined_table &) {
        status_ = STATUS_HAVE_ERROR;
      } catch (const std::exception &e) {
        error_.SetError(ERROR_DB_CONNECTION,
            "Подключение к БД: exception. Запрос:\n" + create_ss.str()
            + "\nexception what: " + e.what());
        status_ = STATUS_HAVE_ERROR;
      }
    } else {
      Logging::Append(io_loglvl::debug_logs, "dry_run:\n" + create_ss.str());
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

/*
 * postgresql language

CREATE TABLE so_items (
   item_id INTEGER NOT NULL,
   so_id INTEGER,
   product_id INTEGER,
   qty INTEGER,
   net_price NUMERIC,
   PRIMARY KEY (item_id, so_id),
   FOREIGN KEY (so_id) REFERENCES so_headers (id)
);

 * or add foreign key after creation table:

ALTER TABLE child_table
ADD CONSTRAINT constraint_fk
FOREIGN KEY (c1)
REFERENCES parent_table(p1)
ON DELETE CASCADE;

 * */

mstatus_t DBConnectionPostgre::SetupConnection() {
  auto connect_str = setupConnectionString();
  if (status_ != STATUS_DRY_RUN && status_ != STATUS_HAVE_ERROR) {
    try {
      pconnect_ = std::unique_ptr<pqxx::connection>(
          new pqxx::connection(connect_str));
      if (pconnect_) {
        if (pconnect_->is_open()) {
          status_ = STATUS_OK;
          is_connected_ = true;
          if (ProgramState::Instance().IsDebugMode())
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
    Logging::Append(io_loglvl::debug_logs, connect_str);
  }
  return status_;
}

void DBConnectionPostgre::CloseConnection() {
  if (pconnect_) {
    pconnect_->disconnect();
    is_connected_ = false;
    if (ProgramState::Instance().IsDebugMode())
      Logging::Append(io_loglvl::debug_logs, "Закрытие соединения c БД "
          + parameters_.name);
  }
}

mstatus_t DBConnectionPostgre::IsTableExists(db_table t, bool *is_exists) {
  if (is_connected_ && pconnect_) {
    std::stringstream select_ss;
    select_ss << "SELECT * FROM " << get_table_name(t) << ";";
    if (status_ != STATUS_DRY_RUN && pconnect_->is_open()) {
      try {
        pqxx::work tr(*pconnect_);
        pqxx::result trres(tr.exec(select_ss.str()));
        // в примерах коммитят после запроса
        tr.commit();

        if (!trres.empty()) {
          *is_exists = true;
          // неплохо бы залогировать результат
          // for example:
          //   std::cout << qres.begin()[0].as<std::string>() << std::endl;
          if (ProgramState::Instance().IsDebugMode())
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
    } else {
      Logging::Append(io_loglvl::debug_logs, "dry_run:\n" + select_ss.str());
    }
  }
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
  std::stringstream sstr("CREATE TABLE ");
  sstr << get_table_name(fields.table) << " (";
  // сначала забить все поля
  for (const auto &field : fields.fields) {
    if (!error_.GetErrorCode()) {
      sstr << db_variable_to_string(field) << "\n";
    } else {
      break;
    }
  }
  if (!error_.GetErrorCode()) {
    // REFERENCE
    for (const auto &ref : *fields.ref_strings) {
      sstr << db_reference_to_string(ref);
      if (error_.GetErrorCode())
        break;
    }
    // PRIMARY KEY
    assert(0);
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
      ss << postgresql_impl::str_db_types[dv.type] << " ";
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
    ss << ",";
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
    str += "(" + ref.fname + ") REFERENCE " + get_table_name(ref.foreign_table) +
        " (" + ref.foreign_fname + ")";
    if (ref.has_on_delete)
      str += " ON DELETE " + ref.GetReferenceActString(ref.delete_method);
    if (ref.has_on_update)
      str += " ON UPDATE " + ref.GetReferenceActString(ref.update_method);
  } else {
    error_.SetError(ew,
        "Проверка поля ссылки на другую таблицу завершилось ошибкой");
  }
}
