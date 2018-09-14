#include "gas_description.h"

#include "models_errors.h"
#include "models_math.h"

//=========================================================================
// dyn_parameters
//=========================================================================
dyn_parameters::dyn_parameters(double cp, double cv, double 
    int_eng, parameters pm)
  : heat_cap_vol(cv), heat_cap_pres(cp), internal_energy(int_eng), 
    beta_kr(0.0), parm(pm) {
  Update();
}

dyn_parameters *dyn_parameters::Init(double cp, double cv, double int_eng, parameters pm) {
  // проверка входных данных
  // check input data:
  //  --  больше нуля     --  > 0.0
  //  --  конечны         --  finite
  bool correct_input = is_above0(cp, cv, int_eng);
  correct_input &= is_above0(pm.pressure, pm.temperature, pm.volume);
  if (!correct_input)
    return nullptr;
  return new dyn_parameters(cp, cv, int_eng, pm);
}

// update beta critical
void dyn_parameters::Update() {
  double ai = heat_cap_pres / heat_cap_vol;
  beta_kr = std::pow(2.0 / (ai + 1.0), ai / (ai - 1.0));
}

//=========================================================================
// const_parameters
//=========================================================================
const_parameters::const_parameters(double vk, double pk,
    double tk, double mol, double R, double af)
  : V_K(vk), P_K(pk), T_K(tk), molecularmass(mol),
    R(R), acentricfactor(af) {}

const_parameters
*const_parameters::Init(double vk, double pk, double tk,
    double mol, double af) {
  /// DEVELOP
  ///   Введение переменных 'correct_input' или
  ///     'tempR' может показаться излишним многословием,
  ///     но это облегчает поддержку кода
  bool correct_input = is_above0(vk, pk, tk, mol, af);
  if (!correct_input)
    return nullptr;
  // 8314.4599 - универсальная газовая постоянная
  //           - GAS CONSTANT - physics constant
  double tempR = 8314.4599 / mol;
  return new const_parameters(vk, pk, tk, mol, tempR, af);
}

const_parameters::const_parameters(const const_parameters &cgp) 
  : V_K(cgp.V_K), P_K(cgp.P_K), T_K(cgp.T_K), 
    molecularmass(cgp.molecularmass), R(cgp.R),
    acentricfactor(cgp.acentricfactor) {}

const_parameters const_parameters::operator= (const const_parameters &cgp) {
  return const_parameters(cgp);
}
/*
const_parameters
*const_parameters::Init(std::array<double, 5> vec) {
  return Init(vec[0], vec[1], vec[2], vec[3], vec[4], vec[5], vec[6]);
}
*/

// ================================================================
// check data functions
// ================================================================
bool is_valid_cgp(const const_parameters &cgp) {
  if (!is_above0(cgp.acentricfactor, cgp.molecularmass,
      cgp.P_K, cgp.R, cgp.T_K, cgp.V_K)) {
    set_error_code(ERR_INIT_T | ERR_INIT_ZERO_ST);
    return false;
  }
  return true;
}

bool is_valid_dgp(const dyn_parameters &dgp) {
  if (!is_above0(dgp.heat_cap_pres, dgp.beta_kr,
      dgp.heat_cap_vol, dgp.internal_energy)) {
    set_error_code(ERR_INIT_T | ERR_INIT_ZERO_ST);
    return false;
  }
  return true;
}
