#include "db_connection_postgre.h"

#include "common.h"
#include "db_query.h"
#include "file_structs.h"
#include "models_configurations.h"
#include "models_logging.h"

#include <pqxx/pqxx>

#include <map>
#include <sstream>

#include <assert.h>

namespace postgresql_impl {
static std::map<db_type, std::string> str_db_types =
    std::map<db_type, std::string>{
  {db_type::db_type_uuid, "UUID"},
  {db_type::db_type_bool, "BOOL"},
  {db_type::db_type_int, "INTEGER"},
  {db_type::db_type_long, "BIGINT"},
  {db_type::db_type_real, "REAL"},
  {db_type::db_type_date, "DATE"},
  {db_type::db_type_time, "TIME"},
  {db_type::db_type_char_array, "CHAR"},
  {db_type::db_type_text, "TEXT"},
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

void DBConnectionPostgre::CreateTable(db_table t,
    const std::vector<create_table_variant> &components) {
  (void) t;
  (void) components;
  assert(0);
}

void DBConnectionPostgre::UpdateTable(db_table t,
    const std::vector<create_table_variant> &components) {
  (void) t;
  (void) components;
  assert(0);
}

void DBConnectionPostgre::InsertModelInfo(const model_info &mi) {
  (void) mi;
  assert(0);
}

void DBConnectionPostgre::SelectModelInfo(
    const std::vector<db_variable> &mip) {
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
          if (ProgramState::Instance().IsDebugMode())
            Logging::Append(io_loglvl::debug_logs, "Подключение к БД "
                + parameters_.name);
        } else {
          error_.SetError(ERR_DB_CONNECTION,
              "Подключение к БД не открыто. Запрос:\n" + connect_str);
          status_ = STATUS_HAVE_ERROR;
        }
      } else {
        error_.SetError(ERR_DB_CONNECTION,
            "Подключение к БД: инициализация. Запрос:\n" + connect_str);
        status_ = STATUS_HAVE_ERROR;
      }
    } catch(const std::exception &e) {
      error_.SetError(ERR_DB_CONNECTION,
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
    if (ProgramState::Instance().IsDebugMode())
      Logging::Append(io_loglvl::debug_logs, "Закрытие соединения c БД "
          + parameters_.name);
  }
}

mstatus_t DBConnectionPostgre::IsTableExists(db_table t, bool *is_exists) {
  if (status_ != STATUS_HAVE_ERROR && pconnect_) {
    std::stringstream select_ss;
    select_ss << "SELECT to_regclass('public." <<
        get_table_name(t) << "');" ;
    if (status_ != STATUS_DRY_RUN && pconnect_->is_open()) {
      try {
        pqxx::nontransaction nt(*pconnect_);
        pqxx::result qres(nt.exec(select_ss.str()));
        if (qres.begin() != qres.end()) {
          *is_exists = true;
          // неплохо бы залогировать результат
          // for example:
          //   std::cout << qres.begin()[0].as<std::string>() << std::endl;
          if (ProgramState::Instance().IsDebugMode())
            Logging::Append(io_loglvl::debug_logs, "Ответ на запрос БД:"
                + select_ss.str() + "\n\t" + qres.begin()[0].as<std::string>());
        } else {
          *is_exists = false;
        }
      } catch (const std::exception &e) {
        error_.SetError(ERR_DB_CONNECTION,
            "Подключение к БД: exception. Запрос:\n" + select_ss.str()
            + "\nexception what: " + e.what());
        status_ = STATUS_HAVE_ERROR;
      }
    } else {
      Logging::Append(io_loglvl::debug_logs, select_ss.str());
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

/* можно ещё разнести:
 *   вообще вынести эту функцию в родительский класс */
std::string DBConnectionPostgre::db_variable_to_string(
    const db_variable &dv) {
  std::stringstream ss;
  auto ew = dv.CheckYourself();
  if (ew.GetErrorCode()) {
    ss << dv.fname << " ";
    auto itDBtype = postgresql_impl::str_db_types.find(dv.type);
    if (itDBtype != postgresql_impl::str_db_types.end()) {
      ss << postgresql_impl::str_db_types[dv.type] << " ";
    } else {
      error_.SetError(ERR_DB_VARIABLE,
          "Тип переменной не задан для данной имплементации БД");
    }
    if (dv.flags.is_array)
      ss << "(" << dv.len << ")";
    if (!dv.flags.can_be_null)
      ss << " NOT NULL";
    if (dv.flags.has_default)
      ss << " DEFAULT " << dv.default_str;
  } else {
    error_.SetError(ew.GetErrorCode(), ew.GetMessage());
  }
  return ss.str();
}
