/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#include "program_state.h"

/* ProgramState */
int ProgramState::calc_key = 0;

ProgramState &ProgramState::Instance() {
  static ProgramState state;
  return state;
}

ProgramState::ProgramState()
  : error_(ERROR_SUCCESS_T), program_config_(nullptr) {}

merror_t ProgramState::ReloadConfiguration(
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

int ProgramState::AddCalculationSetup(const calculation_setup &calc_setup) {
  int key = ProgramState::calc_key++;
  calc_setups_.emplace(key, CalculationSetup(calc_setup));
  if (calc_setups_[key].GetError()) {
    calc_setups_.erase(key);
    key = -1;
  }
  return key;
}

#ifdef _DEBUG
int ProgramState::AddCalculationSetup(CalculationSetup &&setup) {
  int key = ProgramState::calc_key++;
  calc_setups_.emplace(key, setup);
  return key;
}
#endif  // _DEBUG

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

const program_configuration ProgramState::GetConfiguration() const {
  return (program_config_) ?
      program_config_->configuration : program_configuration();
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
  config_by_file = std::unique_ptr<ConfigurationByFile<XMLReader>>(
      ConfigurationByFile<XMLReader>::Init(config_filename));
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
  configuration = program_configuration();
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
