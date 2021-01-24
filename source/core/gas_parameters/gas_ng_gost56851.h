/**
 * asp_therm - implementation of real gas equations of state
 * ===================================================================
 * * gas_ng_gost56851 *
 *   Модуль имплементирующий модель сжиженного природного газа
 * по методологии приведённой в ГОСТ 56851.
 * ===================================================================
 *
 * Copyright (c) 2020-2021 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#ifndef _CORE__GAS_PARAMETERS__GAS_NG_GOST56851_H_
#define _CORE__GAS_PARAMETERS__GAS_NG_GOST56851_H_

#include "ErrorWrap.h"
#include "gas_description_static.h"

/**
 * \brief Класс имплементирующий расчёты компрессированных газовых смесей
 *   по ГОСТ 56851
 * */
class GasParametersGost56851Dyn : public GasParameters {
  ADD_TEST_CLASS(GasParametersGost56851DynProxy);
  /**
   * \brief Структура псевдокритических параметров смеси для ГОСТ 56851
   * */
  struct pseudocritic_parameters {
    /// макропараметры - уд. объём, давление, температура
    parameters vpt;
    /// фактор сжимаемости
    double z;
  };

 public:
  static GasParametersGost56851Dyn* Init(gas_params_input gpi);
  void csetParameters(double v, double p, double t, state_phase sp) override;
  double cCalculateVolume(double p, double t) override;

 private:
  GasParametersGost56851Dyn(parameters prs,
                            const_parameters cgp,
                            ng_gost_mix components);

  /**
   * \brief Инициализировать псевдокритические параметры смеси
   * */
  static pseudocritic_parameters calcPseudocriticVPT(ng_gost_mix components);

  /**
   * \brief Проверить соответсвие допустимость применения
   *   модели для параметров p, t
   *
   * \param p Давление
   * \param t Температура
   *
   * \note Установит ошибку при несоответствии границ
   * */
  merror_t inLimits(double p, double t);

 private:
  /**
   * \brief Контейнер компонентов смеси
   * */
  ng_gost_mix components_;
};






#endif  // !_CORE__GAS_PARAMETERS__GAS_NG_GOST56851_H_
