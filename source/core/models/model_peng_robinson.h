#ifndef _CORE__MODELS__MODEL_PENG_ROBINSON_H_
#define _CORE__MODELS__MODEL_PENG_ROBINSON_H_

#include "common.h"
#include "gas_mix_init.h"
#include "model_general.h"

#include <memory>

class Peng_Robinson final: public modelGeneral {
  double model_coef_a_,
         model_coef_b_,
         model_coef_k_;

private:
  Peng_Robinson(const model_input &mi);

  void set_model_coef();
  void set_model_coef(const const_parameters &cp);

protected:
  void update_dyn_params(dyn_parameters &prev_state,
      const parameters new_state) override;
  void update_dyn_params(dyn_parameters &prev_state,
      const parameters new_state, const const_parameters &cp) override;

  double log_pr(double v, bool is_posit);
  double internal_energy_integral(const parameters new_state,
      const parameters prev_state, const double Tk);
  double heat_capac_vol_integral(const parameters new_state,
      const parameters prev_state, const double Tk);
  double heat_capac_dif_prs_vol(const parameters new_state,
      const double Tk, double R);
// sub functions to update parameters
  double get_volume(double p, double t, const const_parameters &cp);
  double get_pressure(double v, double t, const const_parameters &cp);

public:
  static Peng_Robinson *Init(const model_input &mi);
  void DynamicflowAccept(class DerivateFunctor &df);
  bool IsValid() const override;
  double InitVolume(double p, double t,
      const const_parameters &cp) override;
  void SetVolume(double p, double t) override;
  void SetPressure(double v, double t) override;
#ifndef GAS_MIX_VARIANT
  double GetVolume(double p, double t) const override;
  double GetPressure(double v, double t) const override;
#else
  double GetVolume(double p, double t) override;
  double GetPressure(double v, double t) override;
#endif  // !GAS_MIX_VARIANT

  double GetCoefficient_a() const;
  double GetCoefficient_b() const;
  double GetCoefficient_k() const;
};
#endif  // !_CORE__MODELS__MODEL_PENG_ROBINSON_H_
