#ifndef _CORE__MODELS__MODEL_IDEAL_GAS_H_
#define _CORE__MODELS__MODEL_IDEAL_GAS_H_

#include "common.h"
#include "gas_mix_init.h"
#include "model_general.h"

#include <memory>

class Ideal_Gas final: public modelGeneral {
private:
  Ideal_Gas(const model_input &mi);

protected:
  void update_dyn_params(dyn_parameters &prev_state,
      const parameters new_state) override;
  void update_dyn_params(dyn_parameters &prev_state,
      const parameters new_state, const const_parameters &cp) override;

public:
  static Ideal_Gas *Init(const model_input &mi);

  void DynamicflowAccept(DerivateFunctor &df) override;
  bool IsValid() const override;
  double InitVolume(double p, double t,
      const const_parameters &cp) override;
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
