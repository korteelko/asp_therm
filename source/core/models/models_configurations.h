#ifndef _CORE__MODELS__MODELS_CONFIGURATIONS_H_
#define _CORE__MODELS__MODELS_CONFIGURATIONS_H_

#include "common.h"
#include "models_errors.h"

#include <memory>
#include <string>
#include <vector>

// #define MODEL_MARK_GENERAL__PSEUDOCRITIC     0x01
// #define MODEL_MARK_REDLICH_KWONG__ORIGIN     0x00
// #define MODEL_MARK_PENG_ROBINSON__ORIGIN     0x00

// #error "" FINISH IT ""


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
 * PARAMETERS:
 * - DEBUG_MODE : BOOL
 * - PSEUDOCRITIC : BOOL
 * - ENABLE_ISO_20765 : BOOL
 * - LOG_LEVEL : INT
 * - MODELS : MODELS_STR[]
 * - GASMIX_COMPONETNTS_FILE : STRING
 * - DB_PATH : STRING
*/

/// структура идентификации модели
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

struct model_configuration {
  // usually models have few implementation
  MODEL_MARKS ml_marks;
  const char *const ml_name;
};


/// singleton of state
class ProgramState {
  /// конфигурация расчёта
  class ProgramConfiguration {
  public:
    ProgramConfiguration();
    ProgramConfiguration(const std::string &config_file);

  private:
    void setDefault();

  private:
    merror_t error_;
    std::string config_file_;
    std::vector<model_str> models_;
  };

public:
  static ProgramState &Instance();
  merror_t ResetConfiguration(const std::string &config_file);
  merror_t SetGasmixFile(const std::string &config_file);

  model_configuration GetConfiguration() const;

private:
  std::unique_ptr<ProgramConfiguration> program_config_;
};
/*
model_conf set_model(rg_model_t ml_type, MODEL_MARKS ml_name);
model_conf set_model(const std::string &filename);
bool is_pseudocritic_set(const model_conf &ml_conf);
bool is_mark_set(const model_conf &ml_conf, MODEL_MARKS mark);
*/
#endif  // !_CORE__MODELS__MODELS_CONFIGURATIONS_H_
