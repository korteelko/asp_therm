#include "models_configurations.h"

#include "configuration_strtpl.h"
#include "file_structs.h"

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
typedef std::function<merror_t(models_configuration *,
    const std::string &value)> update_models_config_f;

merror_t update_debug_mode(models_configuration *mc,
    const std::string &val) {
  return (mc) ? set_bool(val, &mc->is_debug_mode) : ERR_INIT_ZERO_ST;
}
merror_t update_pseudocritical(models_configuration *mc,
     const std::string &val) {
  return (mc) ? set_bool(val, &mc->by_pseudocritic) : ERR_INIT_ZERO_ST;
}
merror_t update_enable_iso_20765(models_configuration *mc,
     const std::string &val) {
  return (mc) ? set_bool(val, &mc->enable_iso_20765) : ERR_INIT_ZERO_ST;
}
merror_t update_log_level(models_configuration *mc,
     const std::string &val) {
  return (mc) ? set_loglvl(val, &mc->log_level) : ERR_INIT_ZERO_ST;
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
  {STRTPL_CONFIG_PSEUDOCRITICAL, {update_pseudocritical}},
  {STRTPL_CONFIG_INCLUDE_ISO_20765, {update_enable_iso_20765}},
  {STRTPL_CONFIG_LOG_LEVEL, {update_log_level}}
};
}  // update_configuration_functional namespace

namespace ns_ucf = update_configuration_functional;

merror_t models_configuration::SetConfigurationParameter(
    const std::string &param_strtpl, const std::string &param_value) {
  if (param_strtpl.empty())
    return ERR_STRTPL_TPLNULL;
  merror_t error = ERR_STRTPL_TPLUNDEF;
  auto it_map = ns_ucf::map_config_fuctions.find(param_strtpl);
  if (it_map != ns_ucf::map_config_fuctions.end())
    error = it_map->second.update(this, param_value);
  return error;
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

merror_t ProgramState::GetErrorCode() const {
  return error_.GetErrorCode();
}

const models_configuration ProgramState::GetConfiguration() const {
  assert(0);
}


ProgramState::ProgramConfiguration::ProgramConfiguration(
    const std::string &config_file)
  : error(ERR_SUCCESS_T), config_file(config_file), is_initialized(false) {}
