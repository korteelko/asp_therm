/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020-2021 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#ifndef _CORE__COMMON__MODELS_MATH_H_
#define _CORE__COMMON__MODELS_MATH_H_

#include "Common.h"
#include "ErrorWrap.h"

#include <cmath>
#include <complex>
#include <string>
#include <vector>

#include <assert.h>

inline bool is_above0(double a) {
  return (a > DOUBLE_ACCURACY) && std::isfinite(a);
}

template <class... Targs>
bool is_above0(double a, Targs... fargs) {
  return (is_above0(a) && is_above0(fargs...));
}

/**
 *
 * \brief Метод Кардано/Виета/Тарталья
 *
 * Метод вычисляет корни уравнения
 *   a*x^3 + b*x^2 + c*x + d = 0
 * */
template <
    class T,
    class = typename std::enable_if<std::is_floating_point<T>::value>::type>
merror_t CardanoMethod(const T* coef, std::complex<T>* results) {
  if ((coef == nullptr) || (results == nullptr))
    return ERROR_INIT_NULLP_ST;
  if (coef[0] == 0.0)
    return ERROR_INIT_ZERO_ST;
  const T b = coef[1] / coef[0], c = coef[2] / coef[0], d = coef[3] / coef[0],
          Q = (b * b - 3.0 * c) / 9.0,
          R = (2.0 * b * b * b - 9.0 * b * c + 27.0 * d) / 54.0,
          S = Q * Q * Q - R * R;

  if (S > FLOAT_ACCURACY) {
    T temp = std::acos(R / std::pow(Q, 1.5)) / 3.0;
    results[0] = -2.0 * std::sqrt(Q) * std::cos(temp) - b / 3.0;
    // 2*PI/3 = 2.094395
    results[1] = -2.0 * std::sqrt(Q) * std::cos(2.094395 + temp) - b / 3.0;
    results[2] = -2.0 * std::sqrt(Q) * std::cos(-2.094395 + temp) - b / 3.0;
    return ERROR_SUCCESS_T;

  } else if (S < -FLOAT_ACCURACY) {
    T signR = (std::signbit(R)) ? -1.0 : 1.0;
    if (Q > FLOAT_ACCURACY) {
      T x = std::abs(R / std::pow(Q, 1.5));
      // x < 1.0 - Ошибка реализации
      assert(x >= 1.0);
      T arc = std::log(x + std::sqrt(x * x - 1.0)) / 3.0;
      results[0] = -2.0 * signR * std::sqrt(Q) * std::cosh(arc) - b / 3.0;
      results[1] = signR * std::sqrt(Q) * std::cosh(arc) - b / 3.0;
      results[2] =
          results[1]
          - std::complex<T>(0.0, 1.0) * std::sqrt((T)3.0 * Q) * std::sinh(arc);
      results[1] +=
          std::complex<T>(0.0, 1.0) * std::sqrt((T)3.0 * Q) * std::sinh(arc);

    } else if (Q < -FLOAT_ACCURACY) {
      T x = std::abs(R / std::pow(-Q, 1.5));
      T arc = std::log(x + std::sqrt(x * x + 1.0)) / 3.0;
      results[0] = -2.0 * signR * std::sqrt(-Q) * std::sinh(arc) - b / 3.0;
      results[1] = signR * std::sqrt(-Q) * std::sinh(arc) - b / 3.0;
      results[2] = results[1]
                   - std::complex<T>(0.0, 1.0) * std::sqrt((T)3.0 * (-Q))
                         * std::cosh(arc);
      results[1] +=
          std::complex<T>(0.0, 1.0) * std::sqrt((T)3.0 * (-Q)) * std::cosh(arc);

    } else {
      results[0] = -std::pow(d - b * b * b / 27.0, 0.3333333) - b / 3.0;
      results[1] =
          -(b + results[0]) / (T)2.0
          + std::complex<T>(0.0, 1.0)
                * std::sqrt(std::abs(
                    (b - (T)3.0 * results[0]) * (b + results[0]) - (T)4.0 * c))
                / (T)2.0;
      results[2] =
          results[1]
          - std::complex<T>(0.0, 1.0)
                * std::sqrt(std::abs(
                    (b - (T)3.0 * results[0]) * (b + results[0]) - (T)4.0 * c));
    }
    return ERROR_SUCCESS_T;
  }
  // S == 0
  T signR = (std::signbit(R)) ? -1.0 : 1.0;
  results[0] = -2.0 * signR * std::sqrt(std::abs(Q));
  results[2] = results[1] = -results[0] / T(2.0) - b / T(3.0);
  results[0] -= b / 3.0;
  return ERROR_SUCCESS_T;
}

/**
 * \brief Метод Кардано/Виета/Тарталья
 *
 * \return возвращает количество действительных корней
 * */
template <
    class T,
    class = typename std::enable_if<std::is_floating_point<T>::value>::type>
merror_t CardanoMethod_roots_count(const T* coef,
                                   T* results,
                                   int* roots_count) {
  std::vector<std::complex<T>> vec(3);
  merror_t error = CardanoMethod(coef, &vec[0]);
  if (!error) {
    results[0] = std::real(vec[0]);
    if (std::abs(std::imag(vec[1])) < FLOAT_ACCURACY) {
      results[1] = std::real(vec[1]);
      results[2] = std::real(vec[2]);
      *roots_count = 3;
    } else {
      // for escape problem with sort result or get max
      results[2] = results[1] = results[0];
      *roots_count = 1;
    }
  }
  return error;
}

#endif  // !_CORE__COMMON__MODELS_MATH_H_
