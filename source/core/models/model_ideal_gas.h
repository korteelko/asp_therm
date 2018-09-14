#ifndef _CORE__MODELS__MODEL_IDEAL_GAS_H_
#define _CORE__MODELS__MODEL_IDEAL_GAS_H_

#include "model_general.h"

#include <memory>

class IdealGas final: public modelGeneral {
private:
  IdealGas(modelName mn, parameters prs, const_parameters cgp,
      dyn_parameters dgp, binodalpoints bp);

  IdealGas(modelName mn, parameters prs, parameters_mix components,
      binodalpoints bp);

public:
  static IdealGas *Init(modelName mn, parameters prs, const_parameters cgp,
      dyn_parameters dgp, binodalpoints bp);

  static IdealGas *Init(modelName mn, parameters prs, parameters_mix components,
      binodalpoints bp);
// ========================== <^__.__^>  ===============================
//  OVERRIDED METHODS
// ========================== <^__.__^>  ===============================

  void update_dyn_params(dyn_parameters &prev_state,
      const parameters new_state) override;
  
  bool IsValid() const override;
  void DynamicflowAccept(DerivateFunctor &df) override;

//  void setTemperature(double v, double p);

  void SetVolume(double p, double t)      override;
  void SetPressure(double v, double t)    override;
  double GetVolume(double p, double t)    const override;
  double GetPressure(double v, double t)  const override;
};

#endif  // ! _CORE__MODELS__MODEL_IDEAL_GAS_H_
