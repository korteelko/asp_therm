/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#include "models_math.h"

bool is_equal(double a, double b, double accur) {
  return (std::abs(a-b) < accur);
}

bool is_equal(std::complex<double> a, std::complex<double> b, double accur) {
  return is_equal(a.real(), b.real(), accur) &&
      is_equal(a.imag(), b.imag(), accur);
}

// проверка чисел с плавающей точкой
// check floating variable
//  > 0
bool is_above0(double a) {
  return (a > 0.0 && std::isfinite(a));
}
