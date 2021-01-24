/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020-2021 Mishutinski Yurii
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
#define GASMIX_PERCENT_EPS  0.03

/**
 * \brief Параметры компонента газовой смеси описанные
 *   в файле ининициализации смеси
 * */
struct gasmix_component_info {
  std::string name;
  std::string path;
  double part;

public:
  gasmix_component_info(const std::string &name,
      const std::string &path, const double part);
};
bool operator<(const gasmix_component_info &lg, const gasmix_component_info &rg);


/**
 * \brief Функции расчёта средних параметров по методам из
 *   книги "Свойства газов и жидкостей" Рида, Праусница, Шервуда
 * */
namespace ns_avg {
/* методика применяемая к классической модели Редлиха-Квонга */
/** \brief Рассчитать среднюю критическую температуру по
  *   методу Редлиха-Квонга(двухпараметрическому, глава 4.3) */
double rk2_avg_Tk(const parameters_mix &components);
/** \brief Рассчитать среднее критическое давление по
  *   методу Редлиха-Квонга(двухпараметрическому, глава 4.3) */
double rk2_avg_Pk(const parameters_mix &components);
/** \brief Параметр сжимаемости в критической точке -
  *   для модели Редлиха Квонга равен 1/3 */
double rk2_avg_Zk();
/** \brief Рассчитать среднее значение фактора ацентричности(глава 4.2) */
/* todo: такс, вероятно, там объёмная доля, а не молярная  */
double rk2_avg_acentric(const parameters_mix &components);

// "истинные" параметры критической точки
/** \brief Рассчитать среднюю критическую температуру по
  *   методу Ли (глава 5.7):
  * PSY_i = y_i * Vk_i / SUM_j (y_j * Vk_j)
  * Tk = SUM_i (PSY_i * Tk_i) */
double lee_avg_Tk(const parameters_mix &components);
/** \brief Рассчитать среднюю критическую температуру по
  *   методу Чью-Праусница (глава 5.7):
  * TETA_i = y_i * Vk_i^0.66(6) / SUM_j (y_j * Vk_j^0.66(6))
  * Tk = SUM_i (TETA_i * Tk_i) + SUM_i (SUM_j (TETA_i * TETA_j  * t_i_j))
  *   t_i_j = 0 для i = j
  *   иначе:
  *   psy = A + B*d + C*d^2 + D*d^3 + E*d^4,
  *   also: psy = 2 * t_i_j / (Tk_i + Tk_j)
  *   and: d = |(Tk_i - Tk_j) / (Tk_i + Tk_j)| */
double ch_pr_avg_Tk(const parameters_mix &components);
/** \brief Рассчитать среднюю критический объём смеси по
  *   методу Чью-Праусница (глава 5.7):
  * TETA_i = y_i * Vk_i^0.66(6) / SUM_j (y_j * Vk_j^0.66(6))
  * Vk = SUM_i (TETA_i * Vk_i) + SUM_i (SUM_j (TETA_i * TETA_j  * v_i_j))
  *   v_i_j = 0 для i = j
  *   иначе:
  *   psy = A + B*d + C*d^2 + D*d^3 + E*d^4,
  *   also: psy = 2 * v_i_j / (Vk_i + Vk_j)
  *   and: d = |(Vk_i^0.6667 - Vk_j^0.6667) / (Vk_i^0.6667 + Vk_j^0.6667)| */
double ch_pr_avg_Vk(const parameters_mix &components);
/* todo: вычисление критического давления по книге РПШ
 *   сложно и не понятно */
}  // namespace ns_avg


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
  *   mol = SUM y_i * mol_i  - молярная масса
  *   R = Rm / mol - газовая постоянная.
  * Для других параметров(например критических t, p)
  *   существует несколько методик расчёта, см код
  * Также, в большинстве моделей оперируют не средними параметрами,
  *   а параметрами компонентов */
class GasParameters_mix_dyn final: public GasParameters_mix {
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
  static std::unique_ptr<const_parameters> GetAverageParams(
      parameters_mix &components, const model_str &mi);

  void InitDynamicParams();
  const parameters_mix &GetComponents() const;
  void csetParameters(double v, double p, double t, state_phase sp) override;
};
#endif  // !_CORE__GAS_PARAMETERS__GASMIX_INIT_H_
