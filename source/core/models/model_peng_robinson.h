/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020-2021 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#ifndef _CORE__MODELS__MODEL_PENG_ROBINSON_H_
#define _CORE__MODELS__MODEL_PENG_ROBINSON_H_

#include "atherm_common.h"
#include "gasmix_init.h"
#include "model_general.h"

#include <memory>

class Peng_Robinson final: public modelGeneral {
public:
  static Peng_Robinson *Init(const model_input &mi);
  static model_str GetModelShortInfo(const rg_model_id &model_type);

  model_str GetModelShortInfo() const override;

  void DynamicflowAccept(class DerivateFunctor &df) override;
  bool IsValid() const override;
  bool IsValid(parameters prs) const override;
  void SetVolume(double p, double t) override;
  void SetPressure(double v, double t) override;
  double GetVolume(double p, double t) override;
  double GetPressure(double v, double t) override;

  double GetCoefficient_a() const;
  double GetCoefficient_b() const;
  double GetCoefficient_k() const;

private:
  Peng_Robinson(const model_input &mi);

  /** \brief Установить коэфициенты модели model_coef_a_, model_coef_b_ и
    *   model_coef_k_ по параметрам газа parameters_ */
  void set_model_coef();
  /** \brief Установить коэфициенты модели model_coef_a_, model_coef_b_ и
    *   model_coef_k_ по переданным параметрам cp  */
  void set_model_coef(const const_parameters &cp);
  /** \brief Установить коэфициенты модели model_coef_a_, model_coef_b_ и
    *   model_coef_k_ для газовой смеси используя коэффициенты бинарного
    *   взаимодействия компонентов */
  void coefs_by_binary(const model_input &mi);


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

private:
  double model_coef_a_,
         model_coef_b_,
         model_coef_k_;

};
#endif  // !_CORE__MODELS__MODEL_PENG_ROBINSON_H_
