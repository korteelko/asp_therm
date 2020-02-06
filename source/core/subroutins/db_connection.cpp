#include "db_connection.h"

#include "configuration_strtpl.h"
#include "db_query.h"
#include "file_structs.h"

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

/* DBConnection */
DBConnection::DBConnection()
  : status_(STATUS_DEFAULT) {}

DBConnectionPostgre::DBConnectionPostgre() {}

merror_t DBConnectionPostgre::InitConnection(
    const db_parameters &parameters) {
  parameters_ = parameters;
  merror_t err = ERR_DB_CONNECTION;
  status_ = STATUS_DEFAULT;
  try {
    auto query = DBQueryInitConnection(setupConnectionString());
    assert(0);

    status_ = STATUS_OK;
  } catch (const std::exception &e) {
    error_.SetError(ERR_DB_CONNECTION, e.what());
    error_.LogIt();
    status_ = STATUS_HAVE_ERROR;
  }
  return err;
}

void DBConnectionPostgre::Commit() {
  assert(0);
}

void DBConnectionPostgre::CreateTable(db_table t) {
  assert(0);
}

void DBConnectionPostgre::InsertStateLog(
    const state_log &sl) {
  assert(0);
}

void DBConnectionPostgre::UpdateStateLog(
    const state_log &sl) {
  assert(0);
}

std::string DBConnectionPostgre::setupConnectionString() {

}
