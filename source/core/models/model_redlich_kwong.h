#ifndef _CORE__MODELS__MODEL_REDLICH_KWONG_H_
#define _CORE__MODELS__MODEL_REDLICH_KWONG_H_

#include "common.h"
#include "gasmix_init.h"
#include "model_general.h"

#include <memory>

class Redlich_Kwong2 final: public modelGeneral {
  double model_coef_a_,
         model_coef_b_;

private:
  Redlich_Kwong2(const model_input &mi);

  void set_model_coef();
  void set_model_coef(const const_parameters &cp);
// #ifdef BY_PSEUDO_CRITIC
  model_input set_pseudo_critic_parameters(const model_input &mi);
// #endif  // BY_PSEUDO_CRITIC

protected:
  void update_dyn_params(dyn_parameters &prev_state,
      const parameters new_state) override;
  void update_dyn_params(dyn_parameters &prev_state,
      const parameters new_state, const const_parameters &cp) override;

// integrals for calculating u, cv and cv
  double internal_energy_integral(const parameters new_state,
      const parameters prev_state);
  double heat_capac_vol_integral(const parameters new_state,
      const parameters prev_state);
  double heat_capac_dif_prs_vol(const parameters new_state, double R);
// sub functions to update parameters
  double get_volume(double p, double t, const const_parameters &cp);
  double get_pressure(double v, double t, const const_parameters &cp);

public:
  static Redlich_Kwong2 *Init(const model_input &mi);

  void DynamicflowAccept(class DerivateFunctor &df) override;
  bool IsValid() const override;
  double InitVolume(double p, double t,
      const const_parameters &cp) override;
  void SetVolume(double p, double t)      override;
  void SetPressure(double v, double t)    override;
#ifndef GAS_MIX_VARIANT
  double GetVolume(double p, double t)    const override;
  double GetPressure(double v, double t)  const override;
#else
  double GetVolume(double p, double t)    override;
  double GetPressure(double v, double t)  override;
#endif  // !GAS_MIX_VARIANT

  double GetCoefficient_a() const;
  double GetCoefficient_b() const;
};
#endif  // !_CORE__MODELS__MODEL_REDLICH_KWONG_H_
