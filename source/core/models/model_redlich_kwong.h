#ifndef _CORE_MODELS__MODEL_REDLICH_KWONG_H_
#define _CORE_MODELS__MODEL_REDLICH_KWONG_H_

#include "gas_mix_init.h"
#include "model_general.h"

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

protected:
  void update_dyn_params(dyn_parameters &prev_state,
      const parameters new_state) override;

// integrals for calculating u, cv and cv
  double internal_energy_integral(const parameters state);
  double heat_capac_vol_integral(const parameters state);
  double heat_capac_dif_prs_vol(const parameters state);

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
  double GetVolume(double p, double t)    const override;
  double GetPressure(double v, double t)  const override;

  double GetCoefficient_a() const;
  double GetCoefficient_b() const;
};

#endif  // ! _CORE_MODELS__MODEL_REDLICH_KWONG_H_
