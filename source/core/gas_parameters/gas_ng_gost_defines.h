#ifndef _CORE__GAS_PARAMETERS__GAS_NG_GOST_DEFINES_H_
#define _CORE__GAS_PARAMETERS__GAS_NG_GOST_DEFINES_H_

#include "gas_description.h"
// Данные из ГОСТ 30319.3_2015

struct component_characteristic {
  const gas_t gas_name;
  const double M,  // molar_mass
               Z,  // compress_coef - для НФУ
               E,  // energy_param
               K,  // size_param
               G,  // orintation_param
               Q,  // quadrup_param
               F,  // high_temp_param
               S,  // gipol_param
               W;  // associate_param
};

struct binary_associate_coef {
  const gas_t i,
              j;
  const double E,
               V,
               K,
               G;
};

struct A0_3_coef {
  const double a,
               b,
               c,
               k,
               u,
               g,
               q,
               t,
               s,
               w;
};

#endif  // !_CORE__GAS_PARAMETERS__GAS_NG_GOST_DEFINES_H_