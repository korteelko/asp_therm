/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#ifndef _CORE__GAS_PARAMETERS__GAS_NG_GOST_H_
#define _CORE__GAS_PARAMETERS__GAS_NG_GOST_H_

#include "ErrorWrap.h"
#include "gas_description_static.h"

#include <vector>


// Размерности, константы, параметры при НФУ см. в первой части ГОСТ 30319,
//   т.е. в 30319.1-2015. Коэффициенты в третьей части(30319.3-2015)
struct ng_gost_params {
  /* Параметры используемые ГОСТ моделью */
  double A0,
         A1,
         A2,
         A3;
         /// фактор сжимаемости
  double z,
         /// показатель адиабаты
         k,
         /// Скорорсть звука
         u,
         /// Изобарная теплоёмкость (в идеальном состоянии)
         cp0r,
         /// Динамическая вязкость
         mu;
#if defined(ISO_20765)
  /* Параметры используемые в ISO 20765 */
  /// Свободная энергия Гельмгольца для идеального газа
  double fi0r,
  /// Свободная энергия Гельмгольца
         fi;
#endif  // ISO_20765
};


// const_dyn_parameters init_natural_gas(const gost_ng_components &comps);
class GasParametersGost30319Dyn: public GasParameters {
  ADD_TEST_CLASS(GasParameters_NG_Gost_dynProxy);

public:
  static GasParametersGost30319Dyn *Init(gas_params_input gpi, bool useISO);
  void csetParameters(double v, double p, double t, state_phase) override;
  double cCalculateVolume(double p, double t) override;

  /**
   * \brief Проверить текущие параметры смеси
   * */
  bool IsValid();
  /**
   * \brief Проверить допустимость использования
   *   параметров prs
   * */
  bool IsValid(parameters prs);

private:
  GasParametersGost30319Dyn(parameters prs, const_parameters cgp,
      dyn_parameters dgp, ng_gost_mix components, bool use_iso);
  /**
   * \brief Инициализировать коэффициенты расчётных функций
   * \note Расчитываются только на старте программы
   * */
  bool setFuncCoefficients();
  /**
   * \brief Рассчить молярную массу смеси, медианные значения
   *   удельной теплоёмкости и свободной энергии
   * \note Расчитываются только на старте программы
   * */
  void setStartCondition();
  // init methods
  merror_t init_kx();
  void set_V();
  void set_Q();
  void set_F();
  void set_G();
  void set_Bn();
  void set_Cn();
  merror_t set_molar_mass();
  void set_p0m();
  merror_t init_pseudocrit_vpte();
  /**
   * \brief Установить нулевое значение удельной теплоёмкости
   * */
  merror_t set_cp0r();
#if defined(ISO_20765)
  /**
   * \brief Установить нулевое значение энергии Гельмгольца
   * */
  merror_t set_fi0r();
#endif  // ISO_20765
  /**
   * \brief Пересчитать параметры газовой смеси для новых значений
   *   давления и температуры
   * */
  merror_t set_volume();
  ///  calculate default value of viscosity(mU0)
  void set_viscosity0();
  // init methods end
  double get_Dn(size_t n) const;
  double get_Un(size_t n) const;
  /**
   * \brief Пересчитать приведённую плотность
   * \return Приведённая плотность
   * */
  double calculate_sigma(double p, double t);
  /**
   * \brief Обновить динамические параметры смеси
   * */
  void update_dynamic();
  /* TODO: add accuracy  */
  merror_t check_pt_limits(double p, double t);
  /**
   * \brief Получить первое приблежение для итерационной процедуры
   *   поиска приведённой плотности
   * */
  double sigma_start(double p, double t) const;
  double calculate_d_sigm(double sigm) const;
  double calculate_A0(double sigm) const;
  double calculate_A1(double sigm) const;
  double calculate_A2(double sigm) const;
  double calculate_A3(double sigm) const;
  //void update_parametrs();

private:
  ng_gost_mix components_;
  parameters pseudocrit_vpte_;
  ng_gost_params ng_gost_params_;
  /**
   * \brief Молярная масса смеси
   * */
  double ng_molar_mass_;
  double coef_kx_;
  double coef_V_,
         coef_Q_,
         coef_F_,
         coef_G_,
         coef_p0m_;
  std::vector<double> Bn_;
  std::vector<double> Cn_;
  /**
   * \brief Расчёт по методике ISO 20765
   * */
  bool use_iso20765_;
};

#endif  // !_CORE__GAS_PARAMETERS__GAS_NG_GOST_H_
