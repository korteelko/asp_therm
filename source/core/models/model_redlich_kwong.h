/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#ifndef _CORE__MODELS__MODEL_REDLICH_KWONG_H_
#define _CORE__MODELS__MODEL_REDLICH_KWONG_H_

#include "common.h"
#include "gasmix_init.h"
#include "model_general.h"

#include <memory>

/** \brief Классическая интерпретация уравнения состояния
  *   Редлиха-Квонга(с sqrt(T) в знаменателе) */
class Redlich_Kwong2 final: public modelGeneral {
public:
  static Redlich_Kwong2 *Init(const model_input &mi);

  model_str GetModelShortInfo() const override;

  void DynamicflowAccept(class DerivateFunctor &df) override;
  bool IsValid() const override;
  void SetVolume(double p, double t) override;
  void SetPressure(double v, double t) override;
  double GetVolume(double p, double t) override;
  double GetPressure(double v, double t) override;

  // todo: udoli
  double GetCoefficient_a() const;
  double GetCoefficient_b() const;

private:
  Redlich_Kwong2(const model_input &mi);

  /** \brief Установить коэфициенты модели model_coef_a_ и model_coef_b_
    *   по параметрам газа parameters_ */
  void set_model_coef();
  /** \brief Установить коэфициенты модели model_coef_a_ и model_coef_b_
    *   по переданным параметрам cp  */
  void set_model_coef(const const_parameters &cp);
  /** \brief Установить коэфициенты модели model_coef_a_ и model_coef_b_
    *   для газовой смеси по классическому методу Редлиха-Квонга */
  void gasmix_model_coefs(const model_input &mi);


  void update_dyn_params(dyn_parameters &prev_state,
      const parameters new_state) override;
  void update_dyn_params(dyn_parameters &prev_state,
      const parameters new_state, const const_parameters &cp) override;

// integrals for calculating u, cv and cv
// todo: how does it work with mixes?
  double internal_energy_integral(const parameters new_state,
      const parameters prev_state);
  double heat_capac_vol_integral(const parameters new_state,
      const parameters prev_state);
  double heat_capac_dif_prs_vol(const parameters new_state, double R);
// sub functions to update parameters
  double get_volume(double p, double t, const const_parameters &cp);
  double get_pressure(double v, double t, const const_parameters &cp);

protected:
  double model_coef_a_,
         model_coef_b_;
};
#endif  // !_CORE__MODELS__MODEL_REDLICH_KWONG_H_
