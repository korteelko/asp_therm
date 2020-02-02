#include "models_configurations.h"

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
/*


private:
  merror_t error_;
  std::unique_ptr<ProgramConfiguration> program_config_;
  std::string gasmix_file;
*/

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
