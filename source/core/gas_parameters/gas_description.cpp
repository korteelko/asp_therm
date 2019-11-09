#include "gas_description.h"

#include "models_errors.h"
#include "models_math.h"

#include <map>


static std::map<std::string, gas_t> gas_names =
    std::map<std::string, gas_t> {
  {"methane", GAS_TYPE_METHANE},
  {"ethane", GAS_TYPE_ETHANE},
  {"propane", GAS_TYPE_PROPANE},
  {"hydrogen_sulfide", GAS_TYPE_HYDROGEN_SULFIDE},
  {"hexane", GAS_TYPE_HEXANE},
  {"nitrogen", GAS_TYPE_NITROGEN},
  {"carbon_dioxide", GAS_TYPE_CARBON_DIOXIDE},
  {"helium", GAS_TYPE_HELIUM},
  {"hydrogen", GAS_TYPE_HYDROGEN},
  {"n_butane", GAS_TYPE_N_BUTANE},
  {"i_butane", GAS_TYPE_I_BUTANE},
  {"n_pentane", GAS_TYPE_N_PENTANE},
  {"i_pentane", GAS_TYPE_I_PENTANE},
  {"oxygen", GAS_TYPE_OXYGEN},
  {"argon", GAS_TYPE_ARGON},
  {"heptane", GAS_TYPE_HEPTANE},
  {"octane", GAS_TYPE_OCTANE}
};

static gas_t valid_gases[] = {
  GAS_TYPE_METHANE,
  GAS_TYPE_ETHANE,
  GAS_TYPE_PROPANE,
  GAS_TYPE_HYDROGEN_SULFIDE,
  GAS_TYPE_HEXANE,
  GAS_TYPE_NITROGEN,
  GAS_TYPE_CARBON_DIOXIDE,
  GAS_TYPE_HELIUM,
  GAS_TYPE_HYDROGEN,
  GAS_TYPE_N_BUTANE,
  GAS_TYPE_I_BUTANE,
  GAS_TYPE_N_PENTANE,
  GAS_TYPE_I_PENTANE,
  GAS_TYPE_OXYGEN,
  GAS_TYPE_ARGON,
  GAS_TYPE_HEPTANE,
  GAS_TYPE_OCTANE,

  GAS_TYPE_UNDEFINED,
  GAS_TYPE_MIX
};

bool is_valid_gas(gas_t gas_name) {
  size_t valid_gases_count = sizeof(valid_gases) / sizeof(*valid_gases);
  for (size_t i = 0; i < valid_gases_count; ++i)
    if (gas_name == valid_gases[i])
      return true;
  return false;
}

gas_t gas_by_name(const std::string &name) {
  return (gas_names.find(name) != gas_names.end()) ?
      gas_names[name] : GAS_TYPE_UNDEFINED;
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
  if (correct_input) {
    if (is_valid_gas(gas_name))
      return new const_parameters(
         gas_name, vk, pk, tk, mol, 1000.0 * GAS_CONSTANT / mol, af);
    set_error_message(ERR_INIT_T, "const_parameters: invalid gas_name");
  } else {
    set_error_message(ERR_INIT_T, "const_parameters: input pars < 0");
  }
  return nullptr;
}

const_parameters::const_parameters(const const_parameters &cgp) 
  : gas_name(cgp.gas_name), V_K(cgp.V_K), P_K(cgp.P_K), T_K(cgp.T_K), 
    molecularmass(cgp.molecularmass), R(cgp.R),
    acentricfactor(cgp.acentricfactor) {}

bool is_valid_cgp(const const_parameters &cgp) {
  if (!is_above0(cgp.acentricfactor, cgp.molecularmass,
      cgp.P_K, cgp.R, cgp.T_K, cgp.V_K))
    return false;
  return is_valid_gas(cgp.gas_name) ? true : false;
}

bool is_valid_dgp(const dyn_parameters &dgp) {
  if (!is_above0(dgp.heat_cap_pres, dgp.beta_kr,
      dgp.heat_cap_vol, dgp.internal_energy))
    return false;
  return true;
}

const_dyn_union::~const_dyn_union() {}
