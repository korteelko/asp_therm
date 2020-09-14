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
/**
 * \brief Структура содержащая параметры состояния
 *   ГОСТ(ISO) модели
 * */
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
         w,
         /// Изобарная теплоёмкость (в идеальном состоянии)
         cp0r,
         /// Динамическая вязкость
         mu;
#if defined(ISO_20765)
  /* Параметры используемые в ISO 20765
   * \note Привёдённая температура для ГОСТ и ИСО
   *   перевёрнуты, т.е. "tISO = 1.0 / tGOST" */
         /// Коэффициент B
  double B;
         /// Свободная энергия Гельмгольца для абстракции `идеального газа`
  double fi0r,
         /// Свободная энергия Гельмгольца для абстракции `идеального газа`
         fi0r_t,
         /// Свободная энергия Гельмгольца для абстракции `идеального газа`
         fi0r_tt;

         /// Свободная энергия Гельмгольца
  double fi,
         /// Производная свободной энергии по температуре
         fi_t,
         /// Вторая производная свободной энергии по температуре
         fi_tt,
         /// Производная свободной энергии по плотности
         fi_d,
         /// Производная функции f = (d^2 * fi_d(d, t)) по пр. плотности, где:
         ///   d(сигма) - приведённая плотность;
         ///   t - приведённая температура;
         ///   fi_d - производная свободной энергии по приведённой плотности
         fi_1,
         /// Производная функции f = (d * fi_d(d, t) / t) по пр. температуре,
         ///   умноженная на `-t^2`. Здесь:
         ///   d(сигма) - приведённая плотность;
         ///   t - приведённая температура;
         ///   fi_d - производная свободной энергии по плотности
         fi_2;
         /// Внутреняя энергия
  double u,
         /// Энтальпия
         h,
         /// Энтропия
         s,
         /// Удельная изохорная теплоёмкость
         cv,
         /// Удельная изобарная теплоёмкость
         cp;
#endif  // ISO_20765
};


// const_dyn_parameters init_natural_gas(const gost_ng_components &comps);
class GasParametersGost30319Dyn: public GasParameters {
  ADD_TEST_CLASS(GasParameters_NG_Gost_dynProxy);

public:
  static GasParametersGost30319Dyn *Init(gas_params_input gpi, bool use_iso);
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
  /**
   * \brief Установить значение молярной массы(ng_molar_mass_)
   *   и газовой постоянной смеси(Rm)
   * */
  merror_t set_molar_data();
  void set_p0m();
  void init_pseudocrit_vpte();
  /**
   * \brief Установить нулевое значение удельной теплоёмкости
   * */
  merror_t set_cp0r();
#if defined(ISO_20765)
  /**
   * \brief Установить нулевое значение энергии Гельмгольца
   *   и её производных
   * */
  merror_t set_fi0r();
#endif  // ISO_20765
  /**
   * \brief Пересчитать параметры газовой смеси для новых значений
   *   давления и температуры
   * */
  merror_t set_volume();
  /**
   * \brief Установить параметры A0, A1, A2, A3
   * \param sigma Приведённая плотность
   * */
  void set_gost_params(double sigma);
  ///  calculate default value of viscosity(mU0)
  void set_viscosity0();
  // init methods end
  double get_Dn(size_t n) const;
  double get_Un(size_t n) const;
#if defined(ISO_20765)
  /**
   * \brief Установить данные для пересчёта параметров
   *   модели по ISO20765
   * */
  void set_iso_params(double sigma);
  /**
   * \brief Рассчитать коэффициент B=B(tau)
   * */
  void set_coefB(double t);
  /**
   * \brief Рассчитать значение свободной энергии
   * */
  void set_fi(double t, double sigma);
  /**
   * \brief Рассчитать значение производных свободной энергии
   * */
  void set_fi_der(double t, double sigma);
#endif  // ISO_20765
  /**
   * \brief Пересчитать приведённую плотность
   * \return Приведённая плотность
   * */
  double calculate_sigma(double p, double t) const;
  /**
   * \brief Обновить динамические параметры смеси
   * */
  void update_dynamic();
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
  /**
   * \brief Получить первое приблежение для итерационной процедуры
   *   поиска приведённой плотности
   * */
  double sigma_start(double p, double t) const;
  double calculate_d_sigm(double t, double p, double sigm) const;
  double calculate_A0(double t, double sigm) const;
  double calculate_A1(double t, double sigm) const;
  double calculate_A2(double t, double sigm) const;
  double calculate_A3(double t, double sigm) const;
  //void update_parametrs();

private:
  ng_gost_mix components_;
  parameters pseudocrit_vpte_;
  ng_gost_params ng_gost_params_;
  /**
   * \brief Молярная масса смеси
   * */
  double ng_molar_mass_;
  /**
   * \brief Газовая постоянная смеси (Rm = R / m),
   *   где R - универсальная газовая постоянная,
   *       m - молярная масса
   * */
  double Rm;
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
