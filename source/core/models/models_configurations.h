#ifndef _CORE__MODELS__MODELS_CONFIGURATIONS_H_
#define _CORE__MODELS__MODELS_CONFIGURATIONS_H_

#include "common.h"

#include <string>

#define MODEL_MARK_GENERAL__PSEUDOCRITIC     0x01

#define MODEL_MARK_REDLICH_KWONG__ORIGIN     0x00

#define MODEL_MARK_PENG_ROBINSON__ORIGIN     0x00

// #error "" FINISH IT ""

struct model_conf {
  // define of model
  rg_model_t ml_type;
  // usually models have few implementation
  MODEL_MARKS ml_marks;
  const char *const ml_name;
};

/*
model_conf set_model(rg_model_t ml_type, MODEL_MARKS ml_name);
model_conf set_model(const std::string &filename);
bool is_pseudocritic_set(const model_conf &ml_conf);
bool is_mark_set(const model_conf &ml_conf, MODEL_MARKS mark);
*/
#endif  // !_CORE__MODELS__MODELS_CONFIGURATIONS_H_
