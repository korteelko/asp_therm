#include "db_connection_postgre.h"

#include "common.h"
#include "db_query.h"
#include "file_structs.h"

#include <pqxx/pqxx>

#include <map>

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
std::string db_variable_to_string(const db_variable &dv) {
  (void) dv;
  assert(0);
  return "";
}
}  // namespace postgresql_impl

DBConnectionPostgre::DBConnectionPostgre()
  : pconnect_(nullptr) {}

mstatus_t DBConnectionPostgre::ExecuteQuery(
    const std::string &query_body) {
  (void) query_body;
  return STATUS_NOT;
}

/* DBConnectionPostgre */
mstatus_t DBConnectionPostgre::CheckConnection(
    const db_parameters &parameters) {
  parameters_ = parameters;
  status_ = STATUS_DEFAULT;
  try {
    setupConnection();
    if (error_.GetErrorCode())
      error_.LogIt();
    closeConnection();
  } catch (const std::exception &e) {
    error_.SetError(ERR_DB_CONNECTION, e.what());
    error_.LogIt();
    status_ = STATUS_HAVE_ERROR;
  }
  return status_;
}

void DBConnectionPostgre::Commit() {
  assert(0);
}

void DBConnectionPostgre::Rollback() {
  assert(0);
}

bool DBConnectionPostgre::IsTableExist(db_table t) {
  (void) t;
  assert(0);
  return true;
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
  closeConnection();
}

void DBConnectionPostgre::setupConnection() {
  auto query = DBQueryCheckConnection(
       dynamic_cast<DBConnection *>(this),
       setupConnectionString());
  pconnect_ = std::unique_ptr<pqxx::connection>(
      new pqxx::connection(query.GetQueryBody()));
  if (pconnect_) {
    if (pconnect_->is_open()) {
      status_ = STATUS_OK;
    } else {
      error_.SetError(ERR_DB_CONNECTION,
          "Подключение к БД не открыто. Запрос:\n" + query.GetQueryBody());
      status_ = STATUS_NOT;
    }
  } else {
    error_.SetError(ERR_DB_CONNECTION,
        "Подключение к БД инициализация. Запрос:\n" + query.GetQueryBody());
    status_ = STATUS_NOT;
  }
}

void DBConnectionPostgre::closeConnection() {
  if (status_ == STATUS_OK && pconnect_)
    pconnect_->disconnect();
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
