#include "gas_description.h"

#include "models_errors.h"
#include "models_math.h"

static gas_t valid_gases[5] = {
  GAS_TYPE_UNDEFINED,
  GAS_TYPE_METHANE,
  GAS_TYPE_ETHANE,
  GAS_TYPE_PROPANE,
  GAS_TYPE_MIX
};

bool is_valid_gas(gas_t gas_name) {
  size_t valid_gases_count = sizeof(valid_gases) / sizeof(*valid_gases);
  for (size_t i = 0; i < valid_gases_count; ++i)
    if (gas_name == valid_gases[i])
      return true;
  return false;
}
// dyn_parameters
dyn_parameters::dyn_parameters(double cv, double cp, double 
    int_eng, parameters pm)
  : heat_cap_vol(cv), heat_cap_pres(cp), internal_energy(int_eng), 
    beta_kr(0.0), parm(pm) {
  Update();
}

dyn_parameters *dyn_parameters::Init(double cv, double cp,
    double int_eng, parameters pm) {
  bool correct_input = is_above0(cp, cv, int_eng);
  correct_input &= is_above0(pm.pressure, pm.temperature, pm.volume);
  if (!correct_input)
    return nullptr;
  return new dyn_parameters(cv, cp, int_eng, pm);
}

// update beta critical
void dyn_parameters::Update() {
  double ai = heat_cap_pres / heat_cap_vol;
  beta_kr = std::pow(2.0 / (ai + 1.0), ai / (ai - 1.0));
}

// const_parameters
const_parameters::const_parameters(gas_t gas_name, double vk, double pk,
    double tk, double mol, double R, double af)
  : gas_name(gas_name), V_K(vk), P_K(pk), T_K(tk), molecularmass(mol),
    R(R), acentricfactor(af) {}

const_parameters *const_parameters::Init(gas_t gas_name,
    double vk, double pk, double tk, double mol, double af) {
  bool correct_input = is_above0(vk, pk, tk, mol, af);
  if (!correct_input) {
    set_error_message(ERR_INIT_T, "const_parameters values are lower then 0.0");
    return nullptr;
  }
  correct_input = is_valid_gas(gas_name);
  if (!correct_input) {
    set_error_message(ERR_INIT_T, "const_parameters invalid gas_name");
    return nullptr;
  }
  // 8314.4599 - универсальная газовая постоянная
  double tempR = 8314.4599 / mol;
  return new const_parameters(gas_name, vk, pk, tk, mol, tempR, af);
}

const_parameters::const_parameters(const const_parameters &cgp) 
  : gas_name(cgp.gas_name), V_K(cgp.V_K), P_K(cgp.P_K), T_K(cgp.T_K), 
    molecularmass(cgp.molecularmass), R(cgp.R),
    acentricfactor(cgp.acentricfactor) {}

// check data functions
bool is_valid_cgp(const const_parameters &cgp) {
  if (!is_above0(cgp.acentricfactor, cgp.molecularmass,
      cgp.P_K, cgp.R, cgp.T_K, cgp.V_K)) {
    set_error_code(ERR_INIT_T | ERR_INIT_ZERO_ST);
    return false;
  }
  return is_valid_gas(cgp.gas_name) ? true : false;
}

bool is_valid_dgp(const dyn_parameters &dgp) {
  if (!is_above0(dgp.heat_cap_pres, dgp.beta_kr,
      dgp.heat_cap_vol, dgp.internal_energy)) {
    set_error_code(ERR_INIT_T | ERR_INIT_ZERO_ST);
    return false;
  }
  return true;
}
