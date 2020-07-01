/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#include "models_configurations.h"

#include "configuration_by_file.h"
#include "configuration_strtpl.h"
#include "file_structs.h"
#include "Logging.h"
#include "model_general.h"

#include <functional>
#include <map>

#include <assert.h>


model_str::model_str(rg_model_id mn, int32_t vmaj, int32_t vmin,
    const std::string &info)
  : model_type(mn), vers_major(vmaj),
    vers_minor(vmin), short_info(info) {}

namespace update_configuration_functional {
typedef std::function<merror_t(program_configuration *,
    const std::string &value)> update_models_config_f;

merror_t update_debug_mode(program_configuration *mc, const std::string &val) {
  return (mc) ? set_bool(val, &mc->calc_cfg.is_debug_mode) : ERROR_INIT_ZERO_ST;
}
merror_t update_rk_soave_mod(program_configuration *mc, const std::string &val) {
  return (mc) ? set_bool(val, &mc->calc_cfg.rk_enable_soave_mod) : ERROR_INIT_ZERO_ST;
}
merror_t update_rk_orig_mod(program_configuration *mc, const std::string &val) {
  return (mc) ? set_bool(val, &mc->calc_cfg.rk_enable_origin_mod) : ERROR_INIT_ZERO_ST;
}
merror_t update_pr_binary_coefs(program_configuration *mc, const std::string &val) {
  return (mc) ? set_bool(val, &mc->calc_cfg.pr_enable_by_binary_coefs) : ERROR_INIT_ZERO_ST;
}
merror_t update_enable_iso_20765(program_configuration *mc, const std::string &val) {
  return (mc) ? set_bool(val, &mc->calc_cfg.enable_iso_20765) : ERROR_INIT_ZERO_ST;
}
merror_t update_log_level(program_configuration *mc, const std::string &val) {
  return (mc) ? set_loglvl(val, &mc->log_level) : ERROR_INIT_ZERO_ST;
}
merror_t update_log_file(program_configuration *mc, const std::string &val) {
  mc->log_file = trim_str(val);
  return ERROR_SUCCESS_T;
}

struct config_setup_fuctions {
  /** \brief функция обновляющая параметр */
  update_models_config_f update;
  // /** \brief функция возвращающая строковые значения */
  // get_strtpl get_str_tpl;
};
static std::map<const std::string, config_setup_fuctions> map_config_fuctions =
    std::map<const std::string, config_setup_fuctions> {
  {STRTPL_CONFIG_DEBUG_MODE, {update_debug_mode}},
  {STRTPL_CONFIG_RK_ORIG_MOD, {update_rk_orig_mod}},
  {STRTPL_CONFIG_RK_SOAVE_MOD, {update_rk_soave_mod}},
  {STRTPL_CONFIG_PR_BINARYCOEFS, {update_pr_binary_coefs}},
  {STRTPL_CONFIG_INCLUDE_ISO_20765, {update_enable_iso_20765}},
  {STRTPL_CONFIG_LOG_LEVEL, {update_log_level}},
  {STRTPL_CONFIG_LOG_FILE, {update_log_file}},
};
}  // update_configuration_functional namespace

namespace ns_ucf = update_configuration_functional;


/* model_priority */
model_priority::model_priority()
  : priority(DEF_PRIOR_MINIMUM), is_specified(false) {}

model_priority::model_priority(priority_var priority)
  : priority(priority), is_specified(true) {}

bool model_priority::operator<(const model_priority &s) {
  return this->priority < s.priority;
}

/* program_configuration */
merror_t program_configuration::SetConfigurationParameter(
    const std::string &param_strtpl, const std::string &param_value) {
  if (param_strtpl.empty())
    return ERROR_STRTPL_TPLNULL;
  merror_t error = ERROR_STRTPL_TPLUNDEF;
  auto it_map = ns_ucf::map_config_fuctions.find(param_strtpl);
  if (it_map != ns_ucf::map_config_fuctions.end())
    error = it_map->second.update(this, param_value);
  return error;
}

program_configuration::program_configuration()
  : calc_cfg(calculation_configuration()),
    log_level(io_loglvl::debug_logs), log_file("") {}

/* model_info */
model_info model_info::GetDefault() {
  return model_info {.short_info = model_str(
      rg_model_id(rg_model_t::EMPTY, MODEL_SUBTYPE_DEFAULT), 1, 0, "")};
}
