/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#include "file_structs.h"

#include "atherm_common.h"
#include "configuration_strtpl.h"
#include "Logging.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>


using namespace asp_db;
std::array<std::string, GAS_NODE_COUNT> gas_node::node_t_list = {
  "gas", "section", "group", "subgroup", "parameter"
};

std::array<std::string, GASMIX_NODE_COUNT> gasmix_node::node_t_list = {
  "gasmix", "group", "parameter"
};

std::array<std::string, CONFIG_NODE_COUNT> config_node::node_t_list = {
  "program_config", "group", "parameter"
};

// config_node
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

/** \brief Набор функций обработки параметров структуры db_parameters */
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
namespace ns_ucf = update_configuration_functional;

config_node::config_node(node_type itype, std::string name)
  : config_node_type(itype), name(name), value("") {}

config_node::config_node(node_type itype, std::string name, std::string value)
  : config_node_type(itype), name(name), value(value) {
#ifdef _DEBUG_SUBROUTINS
  Logging::Append(io_loglvl::debug_logs, "Create confignode: " + value +
      " #" + std::to_string(itype) + name);
#endif  // _DEBUG
}

// static
std::string config_node::get_root_name() {
  return config_node::node_t_list[NODE_T_ROOT];
}

node_type config_node::get_node_type(std::string type) {
  for (uint32_t i = 0; i < config_node::node_t_list.size(); ++i)
    if (config_node::node_t_list[i] == type)
      return i;
  return NODE_T_UNDEFINED;
}

merror_t set_db_parameter(db_parameters *dst, const std::string &param_strtpl,
    const std::string &param_value) {
  if (param_strtpl.empty())
    return ERROR_STRTPL_TPLNULL;
  merror_t error = ERROR_STRTPL_TPLUNDEF;
  auto it_map = ns_ucf::map_dbconfig_fuctions.find(param_strtpl);
  if (it_map != ns_ucf::map_dbconfig_fuctions.end())
    error = it_map->second.update(dst, param_value);
  return error;
}


// gas_node
gas_node::gas_node(node_type itype, std::string name)
  : gas_node_type(itype), name(name), value("") {}

gas_node::gas_node(node_type itype, std::string name, std::string value)
  : gas_node_type(itype), name(name), value(value) {
#ifdef _DEBUG_SUBROUTINS
  Logging::Append(io_loglvl::debug_logs, "Create confignode: " + value +
      " #" + std::to_string(itype) + name);
#endif  // _DEBUG
}

// static
std::string gas_node::get_root_name() {
  return gas_node::node_t_list[NODE_T_ROOT];
}

node_type gas_node::get_node_type(std::string type) {
  for (uint32_t i = 0; i < gas_node::node_t_list.size(); ++i)
    if (gas_node::node_t_list[i] == type)
      return i;
  return NODE_T_UNDEFINED;
}

// gasmix_node
gasmix_node::gasmix_node(node_type itype, std::string name)
  : mix_node_type(itype), name(name) {}

gasmix_node::gasmix_node(node_type itype, std::string name, std::string value)
  : mix_node_type(itype), name(name), value(value) {
#ifdef _DEBUG_SUBROUTINS
  Logging::Append(io_loglvl::debug_logs, "Create confignode: " + value +
      " #" + std::to_string(itype) + name);
#endif  // _DEBUG
}

// static
std::string gasmix_node::get_root_name() {
  return gasmix_node::node_t_list[NODE_T_ROOT];
}

node_type gasmix_node::get_node_type(std::string type) {
  for (uint32_t i = 0; i < gasmix_node::node_t_list.size(); ++i)
    if (gasmix_node::node_t_list[i] == type)
      return i;
  return NODE_T_UNDEFINED;
}

/* PARSE TEMPLATE VALUES */
/* data */
static std::map<const std::string, io_loglvl> map_loglvl_tpls =
    std::map<const std::string, io_loglvl> {
  {STRTPL_LOG_LEVEL_NO_LOGS, io_loglvl::no_log},
  {STRTPL_LOG_LEVEL_ERR, io_loglvl::err_logs},
  {STRTPL_LOG_LEVEL_WARN, io_loglvl::warn_logs},
  {STRTPL_LOG_LEVEL_DEBUG, io_loglvl::debug_logs},
};
static std::map<const std::string, db_client> map_dbclient_tpls =
    std::map<const std::string, db_client> {
  {STRTPL_DB_CLIENT_NOONE, db_client::NOONE},
  {STRTPL_DB_CLIENT_POSTGRESQL, db_client::POSTGRESQL},
};
static std::map<const std::string, rg_model_id> map_model_tpls =
    std::map<const std::string, rg_model_id> {
  {STRTPL_MODEL_IDEAL_GAS, rg_model_id(rg_model_t::IDEAL_GAS, MODEL_SUBTYPE_DEFAULT)},
  {STRTPL_MODEL_REDLICH_KWONG, rg_model_id(rg_model_t::REDLICH_KWONG, MODEL_SUBTYPE_DEFAULT)},
  {STRTPL_MODEL_REDLICH_KWONG_SOAVE, rg_model_id(rg_model_t::REDLICH_KWONG, MODEL_RK_SUBTYPE_SOAVE)},
  {STRTPL_MODEL_PENG_ROBINSON, rg_model_id(rg_model_t::PENG_ROBINSON, MODEL_SUBTYPE_DEFAULT)},
  {STRTPL_MODEL_PENG_ROBINSON_B, rg_model_id(rg_model_t::PENG_ROBINSON, MODEL_PR_SUBTYPE_BINASSOC)},
  {STRTPL_MODEL_GOST, rg_model_id(rg_model_t::NG_GOST, MODEL_SUBTYPE_DEFAULT)},
  {STRTPL_MODEL_GOST, rg_model_id(rg_model_t::NG_GOST, MODEL_GOST_SUBTYPE_ISO_20765)}
};

/* functions */
merror_t set_bool(const std::string &val, bool *ans) {
  std::string trimed_val = trim_str(val);
  merror_t error = ERROR_SUCCESS_T;
  if (strcmp(trimed_val.c_str(), STRTPL_BOOL_TRUE) != 0) {
    if (strcmp(trimed_val.c_str(), STRTPL_BOOL_FALSE) != 0) {
      error = ERROR_STRTPL_VALWRONG;
    } else {
      *ans = false;
    }
  } else {
    *ans = true;
  }
  return error;
}

merror_t set_db_client(const std::string &val, db_client *ans) {
  return set_by_map(map_dbclient_tpls, val, *ans);
}

merror_t set_double(const std::string &val, double *ans) {
  merror_t error = ERROR_SUCCESS_T;
  try {
    *ans = std::stod(val);
  } catch (std::invalid_argument &) {
    error = ERROR_STR_TOINT_ST;
    Logging::Append(io_loglvl::debug_logs, "Ошибка при конвертации "
        "строки к типу с плавающей точкой(double): invalid_argument: " + val);
  } catch (std::out_of_range &) {
    error = ERROR_STR_TOINT_ST;
    Logging::Append(io_loglvl::debug_logs, "Ошибка при конвертации "
        "строки к типу с плавающей точкой(double): out_of_range " + val);
  }
  return error;
}

merror_t set_int(const std::string &val, int *ans) {
  merror_t error = ERROR_SUCCESS_T;
  try {
    *ans = std::stoi(val);
  } catch (std::invalid_argument &) {
    error = ERROR_STR_TOINT_ST;
    Logging::Append(io_loglvl::debug_logs, "Ошибка при конвертации "
        "строки к целочисленному типу: invalid_argument " + val);
  } catch (std::out_of_range &) {
    error = ERROR_STR_TOINT_ST;
    Logging::Append(io_loglvl::debug_logs, "Ошибка при конвертации "
        "строки к целочисленному типу: out_of_range " + val);
  }
  return error;
}

merror_t set_loglvl(const std::string &val, io_loglvl *ans) {
  return set_by_map(map_loglvl_tpls, val, *ans);
}

merror_t set_model(const std::string &val, rg_model_id *ans) {
  return set_by_map(map_model_tpls, val, *ans);
}
