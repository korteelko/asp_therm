#include "models_configurations.h"

#include <functional>
#include <map>

#include <assert.h>
/*
struct model_name {
  const int id;
  const char *const name;
};
// warning 
//  what VS say to this???
model_name model_names[] = {
  {MODEL_IDEAL_GAS, "ideal gas"},
  {MODEL_REDLICH_KWONG, "redlich-kwong"},
  {MODEL_PENG_ROBINSON, "peng-robinson"},
  {MODEL_NG_GOST, "GOST 30319-2015"}
};
*/

namespace update_configuration_functional {
  merror_t update_debug_mode(models_configuration *mc,
      const std::string &val) {
    merror_t error = ERR_INIT_ZERO_ST;
    if (mc) {
      сюда
    }
    return error;
  }

  typedef std::function<merror_t(models_configuration *,
      const std::string &value)> update_models_config_f;
  std::map<const std::string, update_models_config_f> map_models_config_f;
}  // anonymous namespace

merror_t models_configuration::SetConfigurationParameter(
    const std::string &param_str, const std::string &param_value) {
  assert(0);
}

ProgramState &ProgramState::Instance() {
  static ProgramState state;
  return state;
}

ProgramState::ProgramState()
  : error_(ERR_SUCCESS_T), program_config_(nullptr), gasmix_file("") {}

merror_t ProgramState::ResetConfigFile(
    const std::string &config_file) {
  assert(0);
}

merror_t ProgramState::ResetGasmixFile(
    const std::string &gasmix_file) {
  assert(0);
}

bool ProgramState::IsInitialized() const {
  assert(0);
}

merror_t ProgramState::GetError() const {
  return error_;
}

const models_configuration ProgramState::GetConfiguration() const {
  assert(0);
}


ProgramState::ProgramConfiguration::ProgramConfiguration(
    const std::string &config_file)
  : error(ERR_SUCCESS_T), config_file(config_file), is_initialized(false) {}
