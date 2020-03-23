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

#include <functional>
#include <map>

#include <assert.h>


model_str::model_str(rg_model_id mn, int32_t vmaj, int32_t vmin,
    const std::string &info)
  : model_type(mn), vers_major(vmaj),
    vers_minor(vmin), short_info(info) {}

namespace update_configuration_functional {
typedef std::function<merror_t(models_configuration *,
    const std::string &value)> update_models_config_f;

merror_t update_debug_mode(models_configuration *mc, const std::string &val) {
  return (mc) ? set_bool(val, &mc->calc_cfg.is_debug_mode) : ERROR_INIT_ZERO_ST;
}
merror_t update_rk_soave_mod(models_configuration *mc, const std::string &val) {
  return (mc) ? set_bool(val, &mc->calc_cfg.rk_is_soave_mod) : ERROR_INIT_ZERO_ST;
}
merror_t update_pr_binary_coefs(models_configuration *mc, const std::string &val) {
  return (mc) ? set_bool(val, &mc->calc_cfg.pr_by_binary_coefs) : ERROR_INIT_ZERO_ST;
}
merror_t update_enable_iso_20765(models_configuration *mc, const std::string &val) {
  return (mc) ? set_bool(val, &mc->calc_cfg.enable_iso_20765) : ERROR_INIT_ZERO_ST;
}
merror_t update_log_level(models_configuration *mc, const std::string &val) {
  return (mc) ? set_loglvl(val, &mc->log_level) : ERROR_INIT_ZERO_ST;
}
merror_t update_log_file(models_configuration *mc, const std::string &val) {
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
  {STRTPL_CONFIG_RK_SOAVE_MOD, {update_rk_soave_mod}},
  {STRTPL_CONFIG_PR_BINARYCOEFS, {update_pr_binary_coefs}},
  {STRTPL_CONFIG_INCLUDE_ISO_20765, {update_enable_iso_20765}},
  {STRTPL_CONFIG_LOG_LEVEL, {update_log_level}},
  {STRTPL_CONFIG_LOG_FILE, {update_log_file}},
};
}  // update_configuration_functional namespace

namespace ns_ucf = update_configuration_functional;

/* calculation_configuration */
bool calculation_configuration::IsDebug() const {
  return is_debug_mode;
}
bool calculation_configuration::RK_IsOriginMod() const {
  return rk_is_origin_mod;
}
bool calculation_configuration::RK_IsSoaveMod() const {
  return rk_is_soave_mod;
}
bool calculation_configuration::PR_ByBinaryCoefs() const {
  return pr_by_binary_coefs;
}
bool calculation_configuration::EnableISO20765() const {
  return enable_iso_20765;
}

merror_t models_configuration::SetConfigurationParameter(
    const std::string &param_strtpl, const std::string &param_value) {
  if (param_strtpl.empty())
    return ERROR_STRTPL_TPLNULL;
  merror_t error = ERROR_STRTPL_TPLUNDEF;
  auto it_map = ns_ucf::map_config_fuctions.find(param_strtpl);
  if (it_map != ns_ucf::map_config_fuctions.end())
    error = it_map->second.update(this, param_value);
  return error;
}

models_configuration::models_configuration()
  : calc_cfg(calculation_configuration()),
    log_level(io_loglvl::debug_logs), log_file("") {}

ProgramState &ProgramState::Instance() {
  static ProgramState state;
  return state;
}

ProgramState::ProgramState()
  : error_(ERROR_SUCCESS_T), program_config_(nullptr), gasmix_file("") {}

merror_t ProgramState::ResetConfigFile(
    const std::string &config_file) {
  program_config_ = std::unique_ptr<ProgramConfiguration>(
      new ProgramConfiguration(config_file));
  if (program_config_) {
    if (program_config_->error.GetErrorCode()) {
      error_.SetError(program_config_->error.GetErrorCode(),
          "Ошибка инициализации конфига программы");
      error_.LogIt();
    }
  }
  return error_.GetErrorCode();
}

merror_t ProgramState::ResetGasmixFile(
    const std::string &gasmix_file) {
  // assert(0);
}

bool ProgramState::IsInitialized() const {
  return (program_config_) ? program_config_->is_initialized : false;
}

bool ProgramState::IsDebugMode() const {
  return (program_config_) ?
      program_config_->configuration.calc_cfg.is_debug_mode : true;
}

bool ProgramState::IsDryRunDBConn() const {
  return (program_config_) ?
      program_config_->db_parameters_conf.is_dry_run : true;
}

merror_t ProgramState::GetErrorCode() const {
  return error_.GetErrorCode();
}

const models_configuration ProgramState::GetConfiguration() const {
  return (program_config_) ?
      program_config_->configuration : models_configuration();
}

const calculation_configuration ProgramState::GetCalcConfiguration() const {
  return (program_config_) ?
      program_config_->configuration.calc_cfg : calculation_configuration();
}

const db_parameters ProgramState::GetDatabaseConfiguration() const {
  return (program_config_) ?
      program_config_->db_parameters_conf : db_parameters();
}

/* ProgramState::ProgramConfiguration */
using PSConfiguration = ProgramState::ProgramConfiguration;

PSConfiguration::ProgramConfiguration()
  : error(ERROR_SUCCESS_T), config_filename(""), is_initialized(false) {
  setDefault();
}

PSConfiguration::ProgramConfiguration(
    const std::string &config_file)
  : error(ERROR_SUCCESS_T), config_filename(config_file),
    is_initialized(false) {
  ResetConfigFile(config_file);
}

merror_t PSConfiguration::ResetConfigFile(
    const std::string &new_config_filename) {
  config_filename = new_config_filename;
  is_initialized = false;
  config_by_file = std::unique_ptr<ConfigurationByFile>(
      ConfigurationByFile::Init(config_filename));
  if (config_by_file) {
    if (config_by_file->GetErrorWrap().GetErrorCode()) {
      error.SetError(ERROR_INIT_T,
          "Ошибка инициализации конфигурации программы");
      error.LogIt();
    } else {
      initProgramConfig();
      initDatabaseConfig();
      is_initialized = true;
    }
  } else {
    error.SetError(ERROR_FILE_IN_ST,
        "Ошибка чтения файла конфигурации программы");
    error.LogIt();
  }
  return error.GetErrorCode();
}

void PSConfiguration::setDefault() {
  /* конфигурация программы */
  configuration = models_configuration();
  /* конфигурация базы данных */
  db_parameters_conf = db_parameters();
}

void PSConfiguration::initProgramConfig() {
  configuration = config_by_file->GetConfiguration();
  Logging::ResetInstance(
      logging_cfg(configuration.log_level, configuration.log_file));
}

void PSConfiguration::initDatabaseConfig() {
  db_parameters_conf = config_by_file->GetDBConfiguration();
}

model_str PSConfiguration::initModelStr() {
  // assert(0);
}
