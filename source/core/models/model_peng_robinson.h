#ifndef _CORE__MODELS__MODEL_PENG_ROBINSON_H_
#define _CORE__MODELS__MODEL_PENG_ROBINSON_H_

#include "gas_mix_init.h"
#include "model_general.h"

#include <memory>

class Peng_Robinson: public modelGeneral {
  double model_coef_a_,
         model_coef_b_,
         model_coef_k_;

private:
  Peng_Robinson(modelName mn, parameters prs, const_parameters cgp,
      dyn_parameters dgp, binodalpoints bp);
  // Init gas_mix
  Peng_Robinson(modelName mn, parameters prs, parameters_mix components,
      binodalpoints bp);

  void set_model_coef();

protected:
  void update_dyn_params(dyn_parameters &prev_state,
      const parameters new_state) override;

  double internal_energy_integral(const parameters state);
  double heat_capac_vol_integral(const parameters state);
  double heat_capac_dif_prs_vol(const parameters state);

public:
  static Peng_Robinson *Init(modelName mn, parameters prs,
      const_parameters cgp, dyn_parameters dgp, binodalpoints bp);
  // Init gas_mix
  static Peng_Robinson *Init(modelName mn, parameters prs,
      parameters_mix components, binodalpoints bp);

  void DynamicflowAccept(class DerivateFunctor &df);

  bool IsValid() const override;

  void SetVolume(double p, double t)      override;
  void SetPressure(double v, double t)    override;
  double GetVolume(double p, double t)    const override;
  double GetPressure(double v, double t)  const override;

  double GetCoefficient_a() const;
  double GetCoefficient_b() const;
  double GetCoefficient_k() const;
};

#endif  // ! _CORE__MODELS__MODEL_PENG_ROBINSON_H_
