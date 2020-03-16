/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#ifndef _CORE__GAS_PARAMETERS__GASMIX_INIT_H_
#define _CORE__GAS_PARAMETERS__GASMIX_INIT_H_

#include "gas_description_static.h"

/* todo: rename this file */

// Не имеет слысла определять все составляющие газовой
//   смеси. 97% составляющихбудет достаточно
// И не забудем ошибку вычисления, или округления,
//   отведём ей 1%
// Итого проверяем тождество :
//   |0.99 - ${summ_of_components}| < 0.02
#define GASMIX_PERSENT_AVR  0.99
#define GASMIX_PERCENT_EPS  0.02

struct gasmix_file {
  std::string name;
  std::string path;
  double part;

public:
  gasmix_file(const std::string &name,
      const std::string &path, const double part);
};

/* TODO: Проверить использование. Реализация - осттавляет вопросы */
bool operator< (const gasmix_file &lg, const gasmix_file &rg);

/* todo: remove this class */
class GasParameters_mix : public GasParameters {
protected:
  parameters_mix components_;

protected:
  GasParameters_mix(parameters prs, const_parameters cgp,
      dyn_parameters dgp, parameters_mix components);
  virtual ~GasParameters_mix();
};

/** \brief класс инкапсулирующий работы с теплофизическими
  *   параметрами смеси газов.
  * Для смеси выставляются такие параметры(y_i - молярная доля компонента):
  *   mol = SUM y_i*mol_i  - молярная масса
  *   w = SUM y_i * w_i - фактор ацентричности(по Рид,
  *     Праусниц, Шервуд для модели Редлиха-Квонга)
  *   R = Rm / mol - газовая постоянная
  *   и т. д. */
class GasParameters_mix_dyn final : public GasParameters_mix {
  // previous pressure, volume and temperature
  parameters prev_vpte_;
  /** \brief обратный указатель на модель, для использования
    *   функций специализированных в модели */
  modelGeneral *model_;

private:
  GasParameters_mix_dyn(parameters prs, const_parameters cgp,
      dyn_parameters dgp, parameters_mix components, modelGeneral *mg);

public:
  static GasParameters_mix_dyn *Init(gas_params_input gpi, modelGeneral *mg);
  //  неправильно, средние параметры зависят от модели
  static std::unique_ptr<const_parameters> 
      GetAverageParams(parameters_mix &components);

  void InitDynamicParams();
  const parameters_mix &GetComponents() const;
  void csetParameters(double v, double p, double t, state_phase sp) override;
};
#endif  // !_CORE__GAS_PARAMETERS__GASMIX_INIT_H_
