/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#include "db_connection.h"

#include "configuration_strtpl.h"
#include "db_query.h"
#include "file_structs.h"
#include "models_configurations.h"
#include "program_state.h"
#include "Logging.h"

#include <functional>
#include <map>
#include <sstream>

#include <assert.h>


/* todo: в итоге здесь перемешаны модуль конфигурации бд,
 *   и модуль инициализации полей таблиц бд */
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
  dbp->name = trim_str(val);
  return ERROR_SUCCESS_T;
}
merror_t update_db_username(db_parameters *dbp, const std::string &val) {
  dbp->username = trim_str(val);
  return ERROR_SUCCESS_T;
}
merror_t update_db_password(db_parameters *dbp, const std::string &val) {
  dbp->password = trim_str(val);
  return ERROR_SUCCESS_T;
}
merror_t update_db_host(db_parameters *dbp, const std::string &val) {
  dbp->host = trim_str(val);
  return ERROR_SUCCESS_T;
}
merror_t update_db_port(db_parameters *dbp, const std::string &val) {
  return (dbp) ? set_int(val, &dbp->port) : ERROR_INIT_ZERO_ST;
}

struct dbconfig_functions {
  /** \brief функция обновляющая параметр */
  update_dbconfig_f update;
  // /** \brief функция возвращающая строковые значения */
  // get_strtpl get_str_tpl;
};

static std::map<const std::string, dbconfig_functions> map_dbconfig_fuctions =
    std::map<const std::string, dbconfig_functions> {
  {STRTPL_CONFIG_DB_DRY_RUN, {update_db_dry_run}},
  {STRTPL_CONFIG_DB_CLIENT, {update_db_client}},
  {STRTPL_CONFIG_DB_NAME, {update_db_name}},
  {STRTPL_CONFIG_DB_USERNAME, {update_db_username}},
  {STRTPL_CONFIG_DB_PASSWORD, {update_db_password}},
  {STRTPL_CONFIG_DB_HOST, {update_db_host}},
  {STRTPL_CONFIG_DB_PORT, {update_db_port}}
};
}  // namespace update_configuration_functional



/* db_parameters */
namespace ns_ucf = update_configuration_functional;
db_parameters::db_parameters()
  : is_dry_run(true), supplier(db_client::NOONE) {}

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
  : status_(STATUS_DEFAULT), parameters_(parameters), is_connected_(false),
    is_dry_run_(true) {
  // дефайн на гугло десты БД
#if !defined(DATABASE_TEST)
  is_dry_run_ = ProgramState::Instance().IsDryRunDBConn();
#endif  // !DATABASE_TEST
}

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
