#ifndef _CORE__GAS_PARAMETERS__GAS_DESCRIPTION_H_
#define _CORE__GAS_PARAMETERS__GAS_DESCRIPTION_H_

#include "gas_defines.h"

#include <map>
#include <string>
#include <utility>
#include <vector>

#include <stdint.h>

bool is_valid_gas(gas_t gas_name);
gas_t gas_by_name(const std::string &name);

inline double volume_by_compress(double p, double t, double z) {
  return z * GAS_CONSTANT * t / p;
}

inline double compress_by_volume(double p, double t, double v) {
  return v *  p / (GAS_CONSTANT * t);
}

/// Динамические параметры вещества, зависящие от
///   других его параметров
struct dyn_parameters {
  dyn_setup setup;
  double heat_cap_vol,     // heat capacity for volume = const // Cv
         heat_cap_pres,    // heat capacity for pressure = const // Cp
         internal_energy,  //
         beta_kr;          // beta_kr=beta_kr(adiabatic_index)
                           //   = [0.0, ... ,1.0]
                           //   if (pressure_1/pressure_2 < beta_kr):
                           //     flow velocity = const, (maximum)
                           //   else : flow velocity ~ p,
                           //     adiabatic_index and etc
                           //   P.S. look dynamic_modeling*.*

  parameters parm;         // current parameters
  // in future)
  // struct therm_potentials potentials;

private:
  dyn_parameters(dyn_setup setup, double cv, double cp,
      double int_eng, parameters pm);
  void check_setup();

public:
  static dyn_parameters *Init(dyn_setup setup, double cv,
      double cp, double int_eng, parameters pm);
  void Update();
};

/* так хочется успеть сделать, но мало времени */
struct therm_potentials {
  double  // internalenergy,
         Hermholtz_free,
         enthalpy,
         Gibbsfree,
         LandauGrand,
         // entropy not potential but calculating in dynamic have sense
         entropy;
};

/// параметры газа, зависящие от его физической природы и
///   не изменяющиеся при изменении его состояния
struct const_parameters {
  const gas_t gas_name;
  const double V_K,              // K point parameters (critical point)
               P_K,
               T_K,
               Z_K,
               molecularmass,
               R,                // gas constant
               acentricfactor;

private:
  const_parameters(gas_t gas_name, double vk, double pk, double tk,
      double zk, double mol, double R, double af);
  const_parameters &operator= (const const_parameters &) = delete;

public:
  static const_parameters *Init(gas_t gas_name, double vk, double pk,
      double tk, double zk, double mol, double af);
  const_parameters(const const_parameters &cgp);
};

bool is_valid_cgp(const const_parameters &cgp);
bool is_valid_dgp(const dyn_parameters &dgp);

/*
 * mixes defines
 * Смесь газов это по сути дефолтная субстанция, с коей
 *   имеем дело, но всё же матаппарат работы с чистым
 *   вецеством скорее всего пригодится
 */
class modelGeneral;
typedef std::pair<const_parameters, dyn_parameters>
    const_dyn_parameters;
typedef std::multimap<const double, const_dyn_parameters> parameters_mix;

/* mix by gost */
/// vol_part is volume part
typedef double vol_part;
/// pair <gas_t, (double) vol_part>
typedef std::pair<gas_t, vol_part> ng_gost_component;
/// vector of ng_gost_component(<gas_t, vol_part>)
typedef std::vector<ng_gost_component> ng_gost_mix;

/*
 * wrap defines
 */
/**
 implementation of input for gas( -
 - .compononents - parameters_mix;  -
 - .ng_gost_components - ng_gost_mix; -
 - .cdp{.cgp, .dgp} - single const and dynamic  )
*/
union const_dyn_union {
  const parameters_mix *components;
  const ng_gost_mix *ng_gost_components;
  struct {
    const const_parameters *cgp;
    const dyn_parameters *dgp;
  } cdp;
  ~const_dyn_union();
};

struct gas_params_input {
  double p, t;
  const_dyn_union const_dyn;
};

struct calculation_state_log {
  dyn_parameters dyn_pars;    // p, v, t and cp(p,v,t), cv(p,v,t), u(p,v,t)
  double enthalpy;
  std::string state_phase;
};
#endif  // !_CORE__GAS_PARAMETERS__GAS_DESCRIPTION_H_
