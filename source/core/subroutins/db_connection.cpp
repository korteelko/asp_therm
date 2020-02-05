#include "db_connection.h"

#include "configuration_strtpl.h"
#include "file_structs.h"
#include "models_errors.h"

#include <pqxx/pqxx>

#include <functional>
#include <map>

#include <assert.h>

namespace update_configuration_functional {
typedef std::function<merror_t(db_parameters *,
    const std::string &value)> update_dbconfig_f;

merror_t update_db_client(db_parameters *dbp, const std::string &val) {
  return (dbp) ? set_db_client(val, &dbp->supplier) : ERR_INIT_ZERO_ST;
}
merror_t update_db_name(db_parameters *dbp, const std::string &val) {
  dbp->name = val;
  return ERR_SUCCESS_T;
}
merror_t update_db_username(db_parameters *dbp, const std::string &val) {
  dbp->username = val;
  return ERR_SUCCESS_T;
}
merror_t update_db_password(db_parameters *dbp, const std::string &val) {
  dbp->password = val;
  return ERR_SUCCESS_T;
}
merror_t update_db_host(db_parameters *dbp, const std::string &val) {
  dbp->host = val;
  return ERR_SUCCESS_T;
}
merror_t update_db_port(db_parameters *dbp, const std::string &val) {
  return (dbp) ? set_int(val, &dbp->port) : ERR_INIT_ZERO_ST;
}

struct dbconfig_setup_fuctions {
  /** \brief функция обновляющая параметр */
  update_dbconfig_f update;
  // /** \brief функция возвращающая строковые значения */
  // get_strtpl get_str_tpl;
};
static std::map<const std::string, dbconfig_setup_fuctions> map_dbconfig_fuctions =
    std::map<const std::string, dbconfig_setup_fuctions> {
  {STRTPL_CONFIG_DB_CLIENT, {update_db_client}},
  {STRTPL_CONFIG_DB_NAME, {update_db_name}},
  {STRTPL_CONFIG_DB_USERNAME, {update_db_username}},
  {STRTPL_CONFIG_DB_PASSWORD, {update_db_password}},
  {STRTPL_CONFIG_DB_HOST, {update_db_host}},
  {STRTPL_CONFIG_DB_PORT, {update_db_port}}
};
}  // update_configuration_functional namespace

namespace ns_ucf = update_configuration_functional;

merror_t db_parameters::SetConfigurationParameter(
    const std::string &param_strtpl, const std::string &param_value) {
  if (param_strtpl.empty())
    return ERR_STRTPL_TPLNULL;
  merror_t error = ERR_STRTPL_TPLUNDEF;
  auto it_map = ns_ucf::map_dbconfig_fuctions.find(param_strtpl);
  if (it_map != ns_ucf::map_dbconfig_fuctions.end())
    error = it_map->second.update(this, param_value);
  return error;
}

db_parameters::db_parameters()
  : supplier(db_client::NOONE) {}


/* DBConnection */
ErrorWrap DBConnection::error_;
db_parameters DBConnection::parameters_;
bool DBConnection::is_connected_ = false;

DBConnection &DBConnection::Instance() {
  static DBConnection db;
  return db;
}

bool DBConnection::ResetConnect(const db_parameters &parameters) {
  assert(0);
  return DBConnection::is_connected_;
}

bool DBConnection::IsConnected() {return DBConnection::is_connected_;}

merror_t DBConnection::GetErrorCode() {}
std::string DBConnection::GetErrorMessage() {
  return error_.GetMessage();
}

DBConnection::DBConnection() {}


/* DBConnection::DBConnectionInstance */
DBConnectionIns::DBConnectionInstance() {}
