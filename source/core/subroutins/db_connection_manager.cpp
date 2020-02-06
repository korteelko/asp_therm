#include "db_connection_manager.h"

#include <pqxx/pqxx>

#include <assert.h>


db_parameters::db_parameters()
  : supplier(db_client::NOONE) {}


/* DBConnection */
namespace therm_postgresql {
  //class
}  //

ErrorWrap DBConnectionManager::error_;
db_parameters DBConnectionManager::parameters_;
bool DBConnectionManager::is_connected_ = false;

DBConnectionManager &DBConnectionManager::Instance() {
  static DBConnectionManager db;
  return db;
}

bool DBConnectionManager::ResetConnect(const db_parameters &parameters) {
  assert(0);
  return DBConnectionManager::is_connected_;
}

bool DBConnectionManager::IsConnected() {return DBConnectionManager::is_connected_;}

merror_t DBConnectionManager::GetErrorCode() {}
std::string DBConnectionManager::GetErrorMessage() {
  return error_.GetMessage();
}

DBConnectionManager::DBConnectionManager() {}


/* DBConnection::DBConnectionInstance */
DBConnectionIns::DBConnectionCreator() {}
