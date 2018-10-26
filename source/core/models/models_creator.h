#ifndef _CORE__MODELS__MODELS_CREATOR_H_
#define _CORE__MODELS__MODELS_CREATOR_H_

#include "gas_description.h"
#include "filereader.h"
#include "xmlreader.h"
#include "model_general.h"
#include "phase_diagram.h"

class ModelsCreater {
  static modelGeneral *set_model(const model_input &mi,
      double p, double t);
  static model_input set_input(modelName mn, const binodalpoints &bp,
      double p, double t, const parameters_mix &components);

public:
  static modelGeneral *GetCalculatingModel(modelName mn,
      std::vector<gas_mix_file> components, double p, double t);
  static modelGeneral *GetCalculatingModel(modelName mn,
      std::vector<gas_mix_file> components);
};
#endif  // !_CORE__MODELS__MODELS_CREATOR_H_
