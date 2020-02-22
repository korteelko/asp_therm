#include "db_connection.h"

#include "configuration_strtpl.h"
#include "db_query.h"
#include "file_structs.h"

#include <functional>
#include <map>
#include <sstream>

#include <assert.h>


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
  dbp->name = val;
  return ERROR_SUCCESS_T;
}
merror_t update_db_username(db_parameters *dbp, const std::string &val) {
  dbp->username = val;
  return ERROR_SUCCESS_T;
}
merror_t update_db_password(db_parameters *dbp, const std::string &val) {
  dbp->password = val;
  return ERROR_SUCCESS_T;
}
merror_t update_db_host(db_parameters *dbp, const std::string &val) {
  dbp->host = val;
  return ERROR_SUCCESS_T;
}
merror_t update_db_port(db_parameters *dbp, const std::string &val) {
  return (dbp) ? set_int(val, &dbp->port) : ERROR_INIT_ZERO_ST;
}

struct dbconfig_fuctions {
  /** \brief функция обновляющая параметр */
  update_dbconfig_f update;
  // /** \brief функция возвращающая строковые значения */
  // get_strtpl get_str_tpl;
};

static std::map<const std::string, dbconfig_fuctions> map_dbconfig_fuctions =
    std::map<const std::string, dbconfig_fuctions> {
  {STRTPL_CONFIG_DB_DRY_RUN, {update_db_dry_run}},
  {STRTPL_CONFIG_DB_CLIENT, {update_db_client}},
  {STRTPL_CONFIG_DB_NAME, {update_db_name}},
  {STRTPL_CONFIG_DB_USERNAME, {update_db_username}},
  {STRTPL_CONFIG_DB_PASSWORD, {update_db_password}},
  {STRTPL_CONFIG_DB_HOST, {update_db_host}},
  {STRTPL_CONFIG_DB_PORT, {update_db_port}}
};
}  // update_configuration_functional namespace

db_variable::db_variable(std::string fname, db_type type,
    db_variable_flags flags, int len)
  : fname(fname), type(type), flags(flags), len(len) {}

ErrorWrap db_variable::CheckYourself() const {
  ErrorWrap ew;
  if (fname.empty()) {
    // just check that looks __FUNCTION__
    ew.SetError(ERROR_DB_VARIABLE,
        "Имя столбца таблицы базы данных не задано "
        + std::string(__FUNCTION__));
  }
  return ew;
}


std::vector<create_table_variant> table_model_info() {
  std::vector<create_table_variant> vars;
  assert(0);
  return vars;
}

std::vector<create_table_variant> table_calculation_info() {
  std::vector<create_table_variant> vars;
  assert(0);
  return vars;
}

std::vector<create_table_variant> table_calculation_state_log() {
  std::vector<create_table_variant> vars;
  assert(0);
  return vars;
}


db_parameters::db_parameters()
  : is_dry_run(true), supplier(db_client::NOONE) {}

namespace ns_ucf = update_configuration_functional;

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
  : status_(STATUS_DEFAULT), parameters_(parameters),
    is_connected_(false) {}

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
