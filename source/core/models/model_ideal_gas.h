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

  // void set_enthalpy();

public:
  static IdealGas *Init(modelName mn, parameters prs, const_parameters cgp,
      dyn_parameters dgp, binodalpoints bp);

  static IdealGas *Init(modelName mn, parameters prs, parameters_mix components,
      binodalpoints bp);

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
  double GetVolume(double p, double t)   const override;
  double GetPressure(double v, double t) const override;
#else
  double GetVolume(double p, double t)   override;
  double GetPressure(double v, double t) override;
#endif  // !GAS_MIX_VARIANT
};

#endif  // ! _CORE__MODELS__MODEL_IDEAL_GAS_H_
