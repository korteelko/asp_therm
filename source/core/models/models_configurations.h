#ifndef _CORE__MODELS__MODELS_CONFIGURATIONS_H_
#define _CORE__MODELS__MODELS_CONFIGURATIONS_H_

#include "common.h"
#include "db_connection.h"
#include "models_errors.h"
#include "models_logging.h"

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
 * DATABASE {
 *   CLIENT : STRING
 *   NAME : STRING
 *   USERNAME : STRING
 *   PASSWORD : STRING
 *   HOST : STRING
 *   PORT : STRING
 * }
 *
 * PARAMETERS:
 * - DEBUG_MODE : BOOL
 * - PSEUDOCRITIC : BOOL
 * - ENABLE_ISO_20765 : BOOL
 * - LOG_LEVEL : INT
 * - MODELS : MODELS_STR[]
 * - DB_PATH : STRING
*/

/// структура идентификации модели(уравнения реального газа)
struct model_str {
  /// define of model
  rg_model_t ml_type;
  /** subtypenumber - наверное привязаться к
    *   енамам конкретных моделей
    * default 0 */
  int32_t subtype_num;
  std::string name;
  int32_t vers_major;
  int32_t vers_minor;
};

struct models_configuration {
  /** \brief выводить отладочную информацию */
  bool is_debug_mode;
  /** \brief пересчитывать модели по псевдокритическим параметрам */
  bool by_pseudocritic;
  /** \brief использовать 'ISO 20665' поверх 'ГОСТ 30319-3' */
  bool enable_iso_20765;
  /** \brief уровень логирования */
  io_loglvl log_level;
};


/// singleton of state
class ProgramState {
  /// конфигурация расчёта
  class ProgramConfiguration {
  public:
    ProgramConfiguration();

    merror_t ResetConfigFile(const std::string &config_file);

  private:
    /** \brief Установить значения по умолчанию
      * для возможных параметров */
    void setDefault();
    /** \brief Считать и инициализировать конфигурацию
      * коннекта к базе данных */
    db_parameters initDatabaseConfig();
    /** \brief Считать и инициализировать конфигурацию модели */
    model_str initModelStr();

  public:
    merror_t error;
    std::string config_file;
    /** \brief Параметры коннекта к БД */
    db_parameters parameters;
    /** \brief Набор конфигураций уравнений состояния реального газа */
    std::vector<model_str> models;
    bool is_initialized;
  };

public:
  /** \brief синглетончик инст */
  static ProgramState &Instance();

  /** \brief Загрузить или перезагрузить конфигурацию программы */
  merror_t ResetConfigFile(const std::string &config_file);
  /** \brief Загрузить или перезагрузить параметры газовой смеси */
  merror_t ResetGasmixFile(const std::string &gasmix_file);

  /** \brief Проверить ProgramConfiguration */
  bool IsInitialized() const;
  merror_t GetError() const;
  const models_configuration GetConfiguration() const;

private:
  merror_t error_;
  std::unique_ptr<ProgramConfiguration> program_config_;
  std::string gasmix_file;
};

#endif  // !_CORE__MODELS__MODELS_CONFIGURATIONS_H_
