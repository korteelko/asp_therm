#ifndef _CORE__MODELS__MODEL_IDEAL_GAS_H_
#define _CORE__MODELS__MODEL_IDEAL_GAS_H_

#include "common.h"
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
  
  void update_dyn_params(dyn_parameters &prev_state,
      const parameters new_state, const const_parameters &cp) override;

  bool IsValid() const override;
  void DynamicflowAccept(DerivateFunctor &df) override;

//  void setTemperature(double v, double p);

  void SetVolume(double p, double t)      override;
  void SetPressure(double v, double t)    override;
#ifndef GAS_MIX_VARIANT
  virtual double GetVolume(double p, double t)   const = 0;
  virtual double GetPressure(double v, double t) const = 0;
#else
  virtual double GetVolume(double p, double t)   = 0;
  virtual double GetPressure(double v, double t) = 0;
#endif  // !GAS_MIX_VARIANT
};

#endif  // ! _CORE__MODELS__MODEL_IDEAL_GAS_H_
