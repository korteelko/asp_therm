/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#ifndef _CORE__MODELS__MODEL_NG_GOST_H_
#define _CORE__MODELS__MODEL_NG_GOST_H_

#include "atherm_common.h"
#include "gas_ng_gost.h"
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

  static model_str GetModelShortInfo(const rg_model_id &model_type);
  model_str GetModelShortInfo() const override;

  void DynamicflowAccept(class DerivateFunctor &df) override;
  bool IsValid() const override;
  bool IsValid(parameters pars) const override;
  void SetVolume(double p, double t) override;
  void SetPressure(double v, double t) override;
  double GetVolume(double p, double t) override;
  double GetPressure(double v, double t) override;

private:
  /** \brief ссылка на параметры газа(GasParameters)
    *   для работы по методологии ГОСТа 30319
    * \note просто dynamic_cast<GasParameters_NG_Gost_dyn>
    *   чтобы не делать тонну перегрузок базового класса,
    *   т.к. для случая этой модели разница методологическая */
  GasParametersGost30319Dyn *ng_pars;
};

#endif  // !_CORE__MODELS__MODEL_NG_GOST_H_
