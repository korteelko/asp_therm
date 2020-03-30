/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#ifndef _CORE__MODELS__MODELS_CONFIGURATIONS_H_
#define _CORE__MODELS__MODELS_CONFIGURATIONS_H_

#include "common.h"
#include "db_connection.h"
#include "db_connection_manager.h"
#include "ErrorWrap.h"
#include "Logging.h"

#include <ctime>
#include <memory>
#include <string>
#include <vector>


/* CONFIGURATION PARAMETERS (XML or JSON)
 * // MODELS_STR - структура описывающая модель
 * MODELS_STR {
 *   TYPE : INT
 *   SUBTYPE : INT
 *   NAME : STRING
 *   VERSION_MAJOR : INT
 *   VERSION_MINOR : INT
 * }
 *
 * DATABASE_CONFIGURATION {
 *   CLIENT : STRING
 *   // DB_PATH : STRING
 *   NAME : STRING
 *   USERNAME : STRING
 *   PASSWORD : STRING
 *   HOST : STRING
 *   PORT : STRING
 * }
 *
 * PROGRAM_CONFIGURATION:
 * - DEBUG_MODE : BOOL
 * - PSEUDOCRITIC : BOOL
 * - INCLUDE_ISO_20765 : BOOL
 * - LOG_LEVEL : INT
 * - LOG_FILE : STRING
 * - DATABASE : DATABASE_CONFIGURATION
 * // - MODELS : MODELS_STR[] - move to calculation.json
*/

class ConfigurationByFile;

/** \brief Приоритет использования модели(относительно
  *   других моделей
  * \note Для организации нескольких моделей в мапе */
struct model_priority {
private:
  /** \brief Численное значение приоритета [-1, 127]
    *   127 - максимальный;
    *   0 - идеальный газ;
    *   -1 - использование модели недопустимо - неверные
    *     константные данные, например мольный состав. */
  priority_var priority;
  /** \brief Приоритет не специализирован и будет задан
    *   посредством конфигурации модели(установлен приоритет
    *   по умлчанию) */
  bool is_specified;
  /** \brief Проводить расчёты игнорируя ограничения модели
    * \note необходимо, скорее из "онтологических" соображений,
    *   поэтому по умолчанию false :) */
  bool is_forced = false;

public:
  model_priority();
  explicit model_priority(priority_var priority);
  /** \brief Получить числовое значение приоритета */
  priority_var GetPriority() const;
  /** \brief Используется значение не по умолчанию */
  bool IsSpecified() const;
  /** \brief Установлен флаг игнорирования границ применения модели */
  bool IsForced() const;
  /* \brief Использование модели допустимо(пока не определился как
    *   правильно использовать эту функцию) */
  // bool IsAvailableModel() const;

  bool operator<(const model_priority &s);
};
inline priority_var model_priority::GetPriority() const { return priority; }
inline bool model_priority::IsSpecified() const { return is_specified; }
inline bool model_priority::IsForced() const { return is_forced; }
// inline bool model_priority::IsAvailableModel() const { return priority != -1; }


/** \brief структура идентификации модели(уравнения реального газа)
  *   параметры прописываются в классе параметров модели
  *   методом GetModelStr */
struct model_str {
  rg_model_id model_type;
  int32_t vers_major;
  int32_t vers_minor;
  /** \brief информация о модели */
  std::string short_info;

public:
  model_str(rg_model_id ml, int32_t vmaj, int32_t vmin,
      const std::string &info);
};

/** \brief Конфигурация запуска программы */
struct calculation_configuration {
  /** \brief флаг вывода отладочной информации
    * \default true */
  bool is_debug_mode = true;
  /** \brief флаг использования классической модели Редлиха-Квонга
    * \default false */
  bool rk_enable_origin_mod = false;
  /** \brief флаг использования модификации Соаве
    *   для модели Редлиха-Квонга
    * \default true */
  bool rk_enable_soave_mod = true;
  /** \brief флаг инициализации модели Пенга-Робинсона через
    *   коэфициенты бинарного взаимодействия
    * \default true */
  bool pr_enable_by_binary_coefs = true;
  /** \brief флаг использования 'ISO 20665' поверх 'ГОСТ 30319-3'
    *    для ng_gost модели
    * \default true */
  bool enable_iso_20765 = true;

public:
  bool IsDebug() const;
  bool RK_IsEnableOriginMod() const;
  bool RK_IsEnableSoaveMod() const;
  bool PR_IsEnableByBinaryCoefs() const;
  bool IsEnableISO20765() const;
};

/** \brief конфигурация прораммы,
  *   общая для запущенной конфигурации */
struct program_configuration {
public:
  /** \brief информация доступных моделей */
  calculation_configuration calc_cfg;
  /** \brief уровень логирования */
  io_loglvl log_level;
  /** \brief файл логирования */
  std::string log_file;

public:
  program_configuration();
  /** \brief изменить параметр(поле), соответствующий 'param_str',
    *   значение параметра соответствует переданному в строке 'param_value'
    * \param param_strtpl текстовый шаблон поля
    * \param param_value значение параметра */
  merror_t SetConfigurationParameter(const std::string &param_strtpl,
      const std::string &param_value);
};

/** \brief строка для добавления в БД */
struct model_info {
  /** \brief информация о модели */
  model_str short_info;
  // dyn_setup dynamic_vars;
};

/** \brief структура содержащая данные инициализации расчёта */
struct calculation_setup {
  /** \brief файл иниициализации газовой смеси */
  std::string gasmix_file;
  /** \brief файл иниициализации используемых моделей */
  std::string models_file;
  /** \brief файл описывающий области расчёта,
    *   соответствующие им модели */
  std::string calculate_file;
};

/** \brief информация о предыдущих расчётах */
/* todo: сейчас это заглушка */
struct dynamic_data {
  /** \brief не учитывать историю предыдущих расчётов */
  /* true если мы считаем каким-то пошаговым алгоритмом */
  bool have_history = false;

  /* \brief история расчёта */
  // std::vector<log_calc> history;
};


/** \brief конфигурация расчёта */
/* todo: remove to separate file */
class CalculationSetup {
public:
  CalculationSetup();
  CalculationSetup(const calculation_setup &cs);
#ifdef _DEBUG
  merror_t AddModel(std::shared_ptr<modelGeneral> &mg);
#endif  // _DEBUG
  merror_t SetModel(int model_key);
  /** \brief Проверить допустимость валидность используемой
    *   модели current_model_ */
  mstatus_t CheckCurrentModel();
  merror_t GetError() const;

private:
  merror_t init_setup();
  void swap_model();

private:
  ErrorWrap error_;
  mstatus_t status_;
  /** \brief копия установленных параметров */
  parameters params_copy_;
#ifdef _DEBUG
  /** \brief список используемых моделей для расчёта */
  std::multimap<priority_var, std::shared_ptr<modelGeneral>> models_;
#else
  /** \brief список используемых моделей для расчёта */
  std::multimap<priority_var, std::unique_ptr<modelGeneral>> models_;
#endif  // _DEBUG
  /** \brief ссылка на текущую используемую модель */
  modelGeneral *current_model_;
  /** \brief данные инициализации расчёта */
  calculation_setup init_data_;
  /** \brief данные расчётов динамики
    * \note в динамике отслеживается история расчётов,
    *   и вероятна последовательная алгоритмизация */
  dynamic_data dyn_data_;
};
using Calculations = std::map<int, CalculationSetup>;

/* сейчас посто заглушка
 * todo: добавить calculation configuration */
struct calculation_info {
  // я хз чё тут надо, но вроде как инпут прописать
  const calculation_configuration &configuration;
  model_str *model;
  time_t time;
};

/** \brief Класс состояния программы:
  *   конфигурация программы, информация о моделях, подключение к БД,
  *   конфигурации расчётов(области, используемые модели etc) */
class ProgramState {
public:
  /** \brief синглетончик инст */
  static ProgramState &Instance();

  /** \brief Загрузить или перезагрузить конфигурацию программы */
  merror_t ReloadConfiguration(const std::string &config_file);
  /** \brief Добавить сетап расчёта
    * \return id расчётных параметров или -1 в случае ошибки */
  int AddCalculationSetup(const calculation_setup &calc_setup);
#ifdef _DEBUG
  /** \brief Добавить сэтап расчёта в список используемых */
  int AddCalculationSetup(CalculationSetup &&setup);
#endif  // _DEBUG

  /** \brief Конфигурация из файла была загружена
    * \return true да, false нет */
  bool IsInitialized() const;
  /** \brief Приложение запущено в режиме отладки
    * \return true да, false нет */
  bool IsDebugMode() const;
  /** \brief Приложение работает без подключения к бд
    * \return true да, false нет */
  bool IsDryRunDBConn() const;
  /** \brief Получить код ошибки */
  merror_t GetErrorCode() const;
  const program_configuration GetConfiguration() const;
  const calculation_configuration GetCalcConfiguration() const;
  const db_parameters GetDatabaseConfiguration() const;

public:
  /** \brief класс конфигурации расчёта */
  class ProgramConfiguration;

private:
  ProgramState();

private:
  static int calc_key;
  ErrorWrap error_;
  /** \brief набор данных для проведения расчётов */
  /* они не связаны между собой, можно распараллелить */
  Calculations calc_setups_;
  /** \brief конфигурация программы - модели, бд, опции */
  std::unique_ptr<ProgramConfiguration> program_config_;
  /** \brief объект подключения к БД */
  DBConnectionManager db_manager_;
};


/** \brief внктренний(nested) класс конфигурации
  *   в классе состояния программы */
class ProgramState::ProgramConfiguration {
public:
  ProgramConfiguration();
  ProgramConfiguration(const std::string &config_filename);

  merror_t ResetConfigFile(const std::string &new_config_filename);

private:
  /** \brief Установить значения по умолчанию
    *   для возможных параметров */
  void setDefault();
  /** \brief Считать и инициализировать конфигурацию программы */
  void initProgramConfig();
  /** \brief Считать и инициализировать конфигурацию
    *   коннекта к базе данных */
  void initDatabaseConfig();
  /** \brief Считать и инициализировать конфигурацию модели */
  model_str initModelStr();

public:
  ErrorWrap error;
  /** \brief Текущий файл конфигурации */
  std::string config_filename;
  /** \brief Конфигурация программы */
  program_configuration configuration;
  /** \brief Параметры коннекта к БД */
  db_parameters db_parameters_conf;
  /** \brief По-сути - декоратор над объектом чтения xml(или других форматов)
    *   файлов для конфигурации программы */
  std::unique_ptr<ConfigurationByFile> config_by_file;
  /** \brief чтение файла завершилось успешной загрузкой
    *   конфигуции программы */
  bool is_initialized;
};

#endif  // !_CORE__MODELS__MODELS_CONFIGURATIONS_H_
