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
  return (dbp) ? set_bool(val, &dbp->is_dry_run) : ERR_INIT_ZERO_ST;
}
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

std::string get_table_name(db_table dt) {
  std::string name = "";
  switch (dt) {
    case db_table::table_model_info:
      name = "model_info";
      break;
    case db_table::table_calculation_info:
      name = "calculation_info";
      break;
    case db_table::table_calculation_state_log:
      name = "calculation_state_log";
      break;
    default:
      assert(0 && "нужно добавить больше типов таблиц");
  }
  return name;
}

db_variable::db_variable(std::string fname, db_type type,
    db_variable_flags flags, int len)
  : fname(fname), type(type), flags(flags), len(len) {}


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

DBConnection::~DBConnection() {}

mstatus_t DBConnection::GetStatus() const {
  return status_;
}

merror_t DBConnection::GetErrorCode() const {
  return error_.GetErrorCode();
}
