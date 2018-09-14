#ifndef _CORE__MODELS__MODELS_CREATOR_H_
#define _CORE__MODELS__MODELS_CREATOR_H_

#include "gas_description.h"
#include "gas_mix_init.h"
#include "phase_diagram.h"

#include "model_general.h"
#include "model_ideal_gas.h"
#include "model_redlich_kwong.h"
#include "model_peng_robinson.h"

// from olad project
// #include "subroutins/inputdata_by_file.h"

#include <memory>

class Equation_of_state {
protected:
  virtual modelGeneral *GetCalculatingModel(parameters prs,
      const_parameters cgp, dyn_parameters dgp, modelName mn) = 0;

  virtual modelGeneral *GetCalculatingModel(parameters prs,
      parameters_mix &components, modelName mn) = 0;

  virtual ~Equation_of_state();
};

class Ideal_gas_equation: protected Equation_of_state {
public:
   modelGeneral *GetCalculatingModel(parameters prs,
      const_parameters cgp, dyn_parameters dgp, modelName mn);
  modelGeneral *GetCalculatingModel(parameters prs,
      parameters_mix &components, modelName mn);
};

class Redlich_Kwong_equation: protected Equation_of_state {
public:
   modelGeneral *GetCalculatingModel(parameters prs,
      const_parameters cgp, dyn_parameters dgp, modelName mn);
  modelGeneral *GetCalculatingModel(parameters prs,
      parameters_mix &components, modelName mn);
};

class Peng_Robinson_equation: protected Equation_of_state {
public:
   modelGeneral *GetCalculatingModel(parameters prs,
      const_parameters cgp, dyn_parameters dgp, modelName mn);
  modelGeneral *GetCalculatingModel(parameters prs,
      parameters_mix &components, modelName mn);
};

// DEVELOP to new file
/*
//=========================================================================
// Dynamic
//=========================================================================

class GasDynamic {
public:
  balloonFlowDynamic *getFlowCalculator(
      const std::shared_ptr<modelGeneral> &spmg, gasparameters &outballoon,
                                               const float V, const float F);

  void calculation(std::shared_ptr<InputData> &idp);

private:
  std::shared_ptr<InputData> idp_;

private:
  balloonFlowDynamic *setCalculator(std::shared_ptr<modelGeneral> &mg,
                                   std::shared_ptr<gasparameters> &pgp);

  std::shared_ptr<Equation_of_state> setEOS();
};
*/
#endif  // ! _CORE__MODELS__MODELS_CREATOR_H_
