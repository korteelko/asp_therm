#ifndef _CORE__MODELS__MODEL_REDLICH_KWONG_H_
#define _CORE__MODELS__MODEL_REDLICH_KWONG_H_

#include "common.h"
#include "gas_mix_init.h"
#include "model_general.h"

#include <array>
#include <memory>

class Redlich_Kwong2 final: public modelGeneral {
  double model_coef_a_,
         model_coef_b_;

private:
  Redlich_Kwong2(modelName mn, parameters prs,
      const_parameters cgp, dyn_parameters dgp,
      binodalpoints bp);
  // Init gas_mix
  Redlich_Kwong2(modelName mn, parameters prs,
      parameters_mix components, binodalpoints bp);

  void set_model_coef();
  // set temp coefficient for gas_mix initialization
  // TODO: do this method virtual
  void set_model_coef(const const_parameters &cp);

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

public:
  static Redlich_Kwong2 *Init(modelName mn, parameters prs,
      const_parameters cgp, dyn_parameters dgp, binodalpoints bp);
  // Init gas_mix
  static Redlich_Kwong2 *Init(modelName mn, parameters prs,
      parameters_mix components, binodalpoints bp);

  void DynamicflowAccept(class DerivateFunctor &df);

  bool IsValid() const override;

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

#endif  // ! _CORE__MODELS__MODEL_REDLICH_KWONG_H_
