/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#ifndef _CORE__GAS_PARAMETERS__GAS_NG_GOST_DEFINES_H_
#define _CORE__GAS_PARAMETERS__GAS_NG_GOST_DEFINES_H_

#include "gas_description.h"
// Данные из ГОСТ 30319.3_2015

// data structs
struct component_characteristics {
  const gas_t gas_name;
  const double M,  // molar_mass
               E,  // energy_param
               K,  // size_param
               G,  // orintation_param
               Q,  // quadrup_param
               F,  // high_temp_param
               S,  // gipol_param
               W;  // associate_param
};
const component_characteristics *get_characteristics(gas_t gas_name);

struct binary_associate_coef {
  const gas_t i,
              j;
  const double E,
               V,
               K,
               G;
};
const binary_associate_coef *get_binary_associate_coefs(gas_t i, gas_t j);

extern const size_t A0_3_coefs_count;
struct A0_3_coef {
  const double a,
               b,
               c,
               k,
               u,
               g,
               q,
               f,
               s,
               w;
};

struct A4_coef {
  const gas_t gas_name;
  const double
  /* A1 and by ISO 20765 */
               A1,
               A2,
               B,
               C,
               D,
               E,
               F,
               G,
               H,
               I,
               J;
};
const A4_coef *get_A4_coefs(gas_t gas_name);

struct critical_params {
  const gas_t gas_name;
  const double temperature,
               density,
               acentric;
};
const critical_params *get_critical_params(gas_t gas_name);

struct A6_coef {
  const gas_t gas_name;
  const double k0,
               k1,
               k2,
               k3;
};
const A6_coef *get_A6_coefs(gas_t gas_name);

struct A7_coef {
  const double c;
  const int r,
            t;
};

struct A8_coef {
  const gas_t gas_name;
  const double k1,
               k2,
               k3,
               k4,
               k5,
               k6;
};
const A8_coef *get_A8_coefs(gas_t gas_name);

#ifndef ISO_20765
struct A9_molar_mass {
  const gas_t gas_name;
  const double M;  // molar_mass
};
const A9_molar_mass *get_molar_mass(gas_t gas_name);
extern const A9_molar_mass A9_molar_masses[];
#endif  // !ISO_20765

/**
 * \brief Коэффициенты бинарного взаимодействия для компонентов СПГ(ГОСТ 56851)
 * */
struct A2_SPG_coef {
  const gas_t i,
              j;
  const double a,
               b;
};
const A2_SPG_coef *get_A2_SPG_coefs(gas_t i, gas_t j);

/**
 * \brief Коэффициенты показателей безразмерных комплексов А0-А3
 *   для ГОСТ 56851
 * */
struct A3_SPG_coef {
  const double b,
               r,
               t,
               g,
               j,
               alf,
               bet,
               ro,
               gam;
};
// const A3_SPG_coef* get_A3_SPG_coefs(size_t n);

/**
 * \brief Коэффициенты расчёта пси для модели СПГ
 * */
struct A4_SPG_coef {
  static constexpr int num = 6;
  static int delta[num];

  gas_t gas_name;
  double coefs[num];
};

/**
 * \brief Коэффициенты расчёта изобарных теплоёмкостей для модели СПГ
 * */
struct A5_SPG_coef {
  static constexpr int num = 5;

  gas_t gas_name;
  double coefs[num];
};

// data
// todo: удалить, организовать доступ только через функции
/*
extern const component_characteristics gases[];
extern const binary_associate_coef gases_coef[];
extern const A4_coef A4_coefs[];
extern const critical_params A5_critical_params[];
extern const A6_coef A6_coefs[];
extern const A7_coef A7_coefs[];
extern const A8_coef A8_coefs[];
extern const int A8_sigmas[];
*/
extern const A0_3_coef A0_3_coefs[];
extern const A3_SPG_coef A3_SPG_coefs[];

template <class NG_COEF_T>
const NG_COEF_T *get_coefs(const NG_COEF_T *coefs_array,
    size_t array_size, gas_t gas_name) {
  for (size_t i = 0; i < array_size; ++i) {
    if (coefs_array[i].gas_name == gas_name)
      return (coefs_array + i);
  }
  return NULL;
}

// functions
/**
 * \brief Установить значение молярной массы(ng_molar_mass_)
 *   и газовой постоянной смеси(Rm)
 *
 * \return Молярную массу газовой смеси и соответственную молярную газовую
 *   постоянную,
 *   ИЛИ NullObject
 * */
molar_parameters calculate_molar_data(ng_gost_mix components);

#endif  // !_CORE__GAS_PARAMETERS__GAS_NG_GOST_DEFINES_H_
