/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020-2021 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#include "program_state.h"

#include "atherm_db_tables.h"


static AthermDBTables db;

/* ProgramState::ProgramConfiguration */
ProgramConfiguration::ProgramConfiguration()
  : BaseObject(STATUS_DEFAULT), config_filename(""), is_initialized(false) {
  setDefault();
}

ProgramConfiguration::ProgramConfiguration(
    const std::string &config_file)
  : BaseObject(STATUS_DEFAULT), config_filename(config_file),
    is_initialized(false) {
  if (ResetConfigFile(config_file)) {
    status_ = STATUS_HAVE_ERROR;
  }
}

merror_t ProgramConfiguration::ResetConfigFile(
    const std::string &new_config_filename) {
  config_filename = new_config_filename;
  is_initialized = false;
  config_by_file = std::unique_ptr<ConfigurationByFile<XMLReader>>(
      ConfigurationByFile<XMLReader>::Init(config_filename));
  if (is_exists(config_filename)) {
    if (config_by_file) {
      if (config_by_file->GetErrorWrap().GetErrorCode()) {
        error_.SetError(ERROR_INIT_T,
            "Ошибка инициализации конфигурации программы");
      } else {
        initProgramConfig();
        initDatabaseConfig();
        is_initialized = true;
      }
    } else {
      error_.SetError(ERROR_FILE_IN_ST, "Ошибка чтения файла конфигурации "
          "программы для файла" + config_filename);
    }
  } else {
    error_.SetError(ERROR_FILE_EXISTS_ST,
        "Файл не существует: " + config_filename);
  }
  return error_.GetErrorCode();
}

void ProgramConfiguration::setDefault() {
  /* конфигурация программы */
  configuration = program_configuration();
}

void ProgramConfiguration::initProgramConfig() {
  if (config_by_file->HasConfiguration()) {
    configuration = config_by_file->GetConfiguration().value();
    Logging::ResetInstance(
        logging_cfg("", configuration.log_level, configuration.log_file,
        configuration.calc_cfg.is_debug_mode));
  }
}

void ProgramConfiguration::initDatabaseConfig() {
  db_parameters_conf = config_by_file->GetDBConfiguration();
}

/* ProgramState */
ProgramState &ProgramState::Instance() {
  static ProgramState state;
  return state;
}

ProgramState::ProgramState()
  : BaseObject(STATUS_DEFAULT), db_manager_(&db) {}

void ProgramState::SetProgramDirs(const file_utils::FileURLRoot &work_dir,
    const file_utils::FileURLRoot &calc_dir) {
  std::lock_guard<Mutex> lock(ProgramState::state_mutex);
  work_dir_.reset(new file_utils::FileURLRoot(work_dir));
  calc_dir_.reset(new file_utils::FileURLRoot(calc_dir));
  if (work_dir_->IsInitialized() && calc_dir_->IsInitialized()) {
    if (is_status_aval(status_))
      status_ = STATUS_OK;
  } else {
    error_.SetError(ERROR_FILE_EXISTS_ST,
        "Ошибка инициализации корневой директории программы");
  }
}

merror_t ProgramState::ReloadConfiguration(
    const std::string &config_file) {
  std::lock_guard<Mutex> lock(ProgramState::state_mutex);
  if (work_dir_) {
    auto path = work_dir_->CreateFileURL(config_file);
    program_config_.ResetConfigFile(path.GetURL());
    if (program_config_.GetError()) {
      error_.SetError(program_config_.GetError(),
          "Ошибка инициализации конфига программы\n"
          "Сообщение: " + program_config_.GetErrorMessage());
    }
  } else {
    error_.SetError(ERROR_INIT_NULLP_ST,
        "Не инициализирована рабочая директория программы");
  }
  return error_.GetErrorCode();
}

void ProgramState::UpdateDatabaseStructure() {
  if (program_config_.db_parameters_conf) {
    db_manager_.ResetConnectionParameters(
        program_config_.db_parameters_conf.value());
    createAthermTables(db_manager_);
  }
}

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
  if (program_config_.db_parameters_conf)
    return program_config_.db_parameters_conf.value().is_dry_run;
  return true;
}

const program_configuration &ProgramState::GetConfiguration() const {
  return program_config_.configuration;
}

const calculation_configuration &ProgramState::GetCalcConfiguration() const {
  return program_config_.configuration.calc_cfg;
}

const std::optional<asp_db::db_parameters> &ProgramState::GetDatabaseConfiguration() const {
  return program_config_.db_parameters_conf;
}

int ProgramState::AddCalculationSetup(const std::string &filepath) {
  std::lock_guard<Mutex> lock(ProgramState::calc_mutex);
  int key = ProgramState::calc_key++;
  auto res = calc_setups_.emplace(key, CalculationSetup(work_dir_,
      // на нормальном яп такого наверное нельзя написать
      (calc_dir_) ? calc_dir_->CreateFileURL(filepath).GetURL() : filepath));
  if (res.second) {
    // добавили успешно
    if (res.first->second.GetError())
      // если была ошибка, то этот элемент удалить
      calc_setups_.erase(res.first);
  }
  return key;
}

void ProgramState::RunCalculationSetup(int num) {
  ProgramState::calc_mutex.lock();
  // auto cs = calc_setups_[num];
  auto cs = calc_setups_.find(num);
  ProgramState::calc_mutex.unlock();

  if (cs != calc_setups_.end())
    cs->second.Calculate();
}

void ProgramState::RemoveCalculationSetup(int num) {
  std::lock_guard<Mutex> lock(ProgramState::calc_mutex);
  auto cs = calc_setups_.find(num);
  if (cs != calc_setups_.end())
    calc_setups_.erase(cs);
}

// model_str PSConfiguration::initModelStr() {}