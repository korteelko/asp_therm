#include "models_math.h"

bool is_equal(double a, double b, double accur) {
  return (std::abs(a-b) < accur);
}

// проверка чисел с плавающей точкой
// check floating variable
//  > 0
bool is_above0(double a) {
  return (a > 0.0 && std::isfinite(a));
}
