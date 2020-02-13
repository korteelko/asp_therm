#ifndef _CORE__MODELS__MODEL_NG_GOST_H_
#define _CORE__MODELS__MODEL_NG_GOST_H_

#include "common.h"
#include "gasmix_init.h"
#include "model_general.h"

#include <memory>

class NG_Gost final: public modelGeneral {
private:
  NG_Gost(const model_input &mi);

protected:
  void update_dyn_params(dyn_parameters &prev_state,
      const parameters new_state) override;
  void update_dyn_params(dyn_parameters &prev_state,
      const parameters new_state, const const_parameters &cp) override;

public:
  static NG_Gost *Init(const model_input &mi);

  void DynamicflowAccept(class DerivateFunctor &df) override;
  bool IsValid() const override;
  double InitVolume(double p, double t,
      const const_parameters &cp) override;
  void SetVolume(double p, double t) override;
  void SetPressure(double v, double t) override;
  double GetVolume(double p, double t) override;
  double GetPressure(double v, double t) override;
};
#endif  // !_CORE__MODELS__MODEL_NG_GOST_H_
