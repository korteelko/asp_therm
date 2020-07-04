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

#include "atherm_db_tables.h"

static AthermDBTables db;


/* ProgramState */
ProgramState &ProgramState::Instance() {
  static ProgramState state;
  return state;
}

ProgramState::ProgramState()
  : status_(STATUS_DEFAULT), db_manager_(&db) {}

void ProgramState::SetWorkDir(const file_utils::FileURLRoot &orig) {
  std::lock_guard<Mutex> lock(mutex);
  work_dir_.reset(new file_utils::FileURLRoot(orig));
  if (work_dir_->IsInitialized()) {
    if (is_status_aval(status_)) {
      status_ = STATUS_OK;
    }
  } else {
    error_.SetError(ERROR_FILE_EXISTS_ST,
        "Ошибка инициализации корневой директории программы");
  }
}

merror_t ProgramState::ReloadConfiguration(
    const std::string &config_file) {
  std::lock_guard<Mutex> lock(mutex);
  if (work_dir_) {
    auto path = work_dir_->CreateFileURL(config_file);
    program_config_.ResetConfigFile(path.GetURL());
    if (program_config_.error.GetErrorCode()) {
      error_.SetError(program_config_.error.GetErrorCode(),
          "Ошибка инициализации конфига программы\n"
          "Сообщение: " + program_config_.error.GetMessage());
    }
  } else {
    error_.SetError(ERROR_INIT_NULLP_ST,
        "Не инициализирована рабочая директория программы");
  }
  return error_.GetErrorCode();
}

int ProgramState::AddCalculationSetup(const calculation_setup &calc_setup) {
  std::lock_guard<Mutex> lock(ProgramState::mutex);
  int key = ProgramState::calc_key++;
  CalculationSetup &&cs(calc_setup);
  if (!cs.GetError())
    calc_setups_.emplace(key, cs);
  return key;
}

#ifdef _DEBUG
int ProgramState::AddCalculationSetup(CalculationSetup &&setup) {
  std::lock_guard<Mutex> lock(mutex);
  int key = ProgramState::calc_key++;
  calc_setups_.emplace(key, setup);
  return key;
}
#endif  // _DEBUG

bool ProgramState::IsInitialized() const {
  mstatus_t st = status_;
  if (!program_config_.is_initialized)
    st = STATUS_NOT;
  return (error_.GetErrorCode()) ? false : is_status_ok(st);
}

bool ProgramState::IsDebugMode() const {
  return program_config_.configuration.calc_cfg.is_debug_mode;
}

bool ProgramState::IsDryRunDBConn() const {
  return program_config_.db_parameters_conf.is_dry_run;
}

const program_configuration &ProgramState::GetConfiguration() const {
  return program_config_.configuration;
}

const calculation_configuration &ProgramState::GetCalcConfiguration() const {
  return program_config_.configuration.calc_cfg;
}

const asp_db::db_parameters &ProgramState::GetDatabaseConfiguration() const {
  return program_config_.db_parameters_conf;
}

mstatus_t ProgramState::GetStatus() const {
  return status_;
}

merror_t ProgramState::GetErrorCode() const {
  return error_.GetErrorCode();
}

std::string ProgramState::GetErrorMessage() const {
  return error_.GetMessage();
}

void ProgramState::LogError() {
  error_.LogIt();
}

/* ProgramState::ProgramConfiguration */
using PSConfiguration = ProgramState::ProgramConfiguration;

PSConfiguration::ProgramConfiguration()
  : status(STATUS_DEFAULT), config_filename(""), is_initialized(false) {
  setDefault();
}

PSConfiguration::ProgramConfiguration(
    const std::string &config_file)
  : status(STATUS_DEFAULT), config_filename(config_file),
    is_initialized(false) {
  if (ResetConfigFile(config_file)) {
    status = STATUS_HAVE_ERROR;
  }
}

merror_t PSConfiguration::ResetConfigFile(
    const std::string &new_config_filename) {
  config_filename = new_config_filename;
  is_initialized = false;
  config_by_file = std::unique_ptr<ConfigurationByFile<XMLReader>>(
      ConfigurationByFile<XMLReader>::Init(config_filename));
  if (is_exist(config_filename)) {
    if (config_by_file) {
      if (config_by_file->GetErrorWrap().GetErrorCode()) {
        error.SetError(ERROR_INIT_T,
            "Ошибка инициализации конфигурации программы");
      } else {
        initProgramConfig();
        initDatabaseConfig();
        is_initialized = true;
      }
    } else {
      error.SetError(ERROR_FILE_IN_ST, "Ошибка чтения файла конфигурации "
          "программы для файла" + config_filename);
    }
  } else {
    error.SetError(ERROR_FILE_EXISTS_ST,
        "Файл не существует: " + config_filename);
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
      logging_cfg(configuration.log_level, configuration.log_file,
      configuration.calc_cfg.is_debug_mode));
}

void PSConfiguration::initDatabaseConfig() {
  db_parameters_conf = config_by_file->GetDBConfiguration();
}

model_str PSConfiguration::initModelStr() {
  // assert(0);
}
