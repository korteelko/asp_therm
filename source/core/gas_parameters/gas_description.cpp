/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#include "gas_description.h"

#include "models_math.h"

#include <map>

ErrorWrap const_parameters::init_error;

/**
 * \brief список газов, для которых прописаны файлы параметров,
 *   ключ соответствует имени xml файла
 *   (и строке в БД если мне будет не лень)
 * \todo ето надо объединить с вектором `valid_gases`
 * */
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
  {"iso_butane", GAS_TYPE_ISO_BUTANE},
  {"n_pentane", GAS_TYPE_N_PENTANE},
  {"iso_pentane", GAS_TYPE_ISO_PENTANE},
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
  GAS_TYPE_ISO_BUTANE,
  GAS_TYPE_N_PENTANE,
  GAS_TYPE_ISO_PENTANE,
  GAS_TYPE_OXYGEN,
  GAS_TYPE_ARGON,
  GAS_TYPE_HEPTANE,
  GAS_TYPE_OCTANE,

#ifdef GASMIX_TEST
  GAS_TYPE_TOLUENE,
#endif  // GASMIX_TEST

  GAS_TYPE_UNDEFINED,
  GAS_TYPE_MIX
};

static bool is_valid_gas(gas_t gas_name) {
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
dyn_parameters::dyn_parameters(dyn_setup setup, double cv, double cp,
    double int_eng, parameters pm)
  : setup(setup), heat_cap_vol(cv), heat_cap_pres(cp),
    internal_energy(int_eng), beta_kr(0.0), parm(pm) {
  status = STATUS_OK;
  Update();
}

void dyn_parameters::check_setup() {
  if (setup & DYNAMIC_HEAT_CAP_VOL & DYNAMIC_HEAT_CAP_PRES)
    setup |= DYNAMIC_BETA_KR;
}

merror_t dyn_parameters::check_input(dyn_setup setup, double cv, double cp,
    double int_eng, parameters pm) {
  cp = (setup & DYNAMIC_HEAT_CAP_PRES) ? cp : 1.0;
  cv = (setup & DYNAMIC_HEAT_CAP_VOL) ? cv : 1.0;
  int_eng = (setup & DYNAMIC_INTERNAL_ENERGY) ? int_eng : 1.0;
  bool correct_input = is_above0(cp, cv, int_eng);
  // pm.volume мы не проверяем, т.к. можно пересчитать
  correct_input &= is_above0(pm.pressure, pm.temperature);
  if (!correct_input)
    return ERROR_INIT_T;
  return ERROR_SUCCESS_T;
}

dyn_parameters *dyn_parameters::Init(dyn_setup setup, double cv, double cp,
    double int_eng, parameters pm) {
  if (dyn_parameters::check_input(setup, cv, cp, int_eng, pm))
    return nullptr;
  return new dyn_parameters(setup, cv, cp, int_eng, pm);
}

dyn_parameters::dyn_parameters()
  : status(STATUS_NOT), setup(0) {}

merror_t dyn_parameters::ResetParameters(dyn_setup new_setup, double cv,
    double cp, double int_eng, parameters pm) {
  merror_t error = dyn_parameters::check_input(new_setup, cv, cp, int_eng, pm);
  if (!error) {
    status = STATUS_OK;
    setup = new_setup;
    heat_cap_vol = cv;
    heat_cap_pres = cp;
    internal_energy = int_eng;
    beta_kr = 0.0;
    parm = pm;
  }
  return error;
}

// update beta critical
void dyn_parameters::Update() {
  if (status == STATUS_OK) {
    if (setup & DYNAMIC_BETA_KR) {
      double ai = heat_cap_pres / heat_cap_vol;
      beta_kr = std::pow(2.0 / (ai + 1.0), ai / (ai - 1.0));
    }
  }
}

const_parameters::const_parameters(gas_t gas_name, double vk, double pk,
    double tk, double zk, double mol, double R, double af)
  : gas_name(gas_name), V_K(vk), P_K(pk), T_K(tk), Z_K(zk),
    molecularmass(mol), R(R), acentricfactor(af) {}

const_parameters *const_parameters::Init(gas_t gas_name, double vk,
    double pk, double tk, double zk, double mol, double af) {
  bool correct_input = false;
  if (gas_name != GAS_TYPE_MIX) {
    correct_input = const_parameters::check_params(vk, pk, tk, zk, mol, af);
    if (correct_input) {
      if (!is_above0(vk)) {
        if (!is_above0(zk)) {
          correct_input = false;
        } else {
          vk = volume_by_compress(pk, tk, mol, zk);
        }
      } else {
        zk = compress_by_volume(pk, tk, mol, vk);
      }
    }
  } else {
    // is gasmix
    correct_input = is_above0(mol);
  }
  if (correct_input) {
    if (is_valid_gas(gas_name))
      return new const_parameters(gas_name, vk, pk, tk, zk,
          mol, 1000.0 * GAS_CONSTANT / mol, af);
    const_parameters::init_error.SetError(
        ERROR_INIT_T, "const_parameters: invalid gas_name");
  } else {
    const_parameters::init_error.SetError(
        ERROR_INIT_T, "const_parameters: input pars < 0");
  }
  // залогировать ошивку если она была
  init_error.LogIt();
  return nullptr;
}

// const_parameters
bool const_parameters::check_params(double vk, double pk, double tk,
    double zk, double mol, double af) {
  // acentic factor can be(for example - helium) < 0.0
  (void) af;
  bool is_valid = is_above0(pk, tk, mol);
  if (is_valid) {
    if (!is_above0(vk)) {
      if (!is_above0(zk)) {
        is_valid = false;
      }
    }
  }
  return is_valid;
}

const_parameters::const_parameters(const const_parameters &cgp) 
  : gas_name(cgp.gas_name), V_K(cgp.V_K), P_K(cgp.P_K), T_K(cgp.T_K), 
    Z_K(cgp.Z_K), molecularmass(cgp.molecularmass), R(cgp.R),
    acentricfactor(cgp.acentricfactor) {}

bool const_parameters::IsGasmix() const {
  return gas_name == GAS_TYPE_MIX;
}

bool const_parameters::IsAbstractGas() const {
  return gas_name == GAS_TYPE_UNDEFINED;
}

bool is_valid_cgp(const const_parameters &cgp) {
  if (cgp.gas_name != GAS_TYPE_MIX) {
    if (!const_parameters::check_params(cgp.V_K, cgp.P_K, cgp.T_K, cgp.Z_K,
        cgp.molecularmass, cgp.acentricfactor) || !is_above0(cgp.R))
      return false;
  }
  return is_valid_gas(cgp.gas_name) ? true : false;
}

bool is_valid_dgp(const dyn_parameters &dgp) {
  if (!is_above0(dgp.heat_cap_pres, dgp.beta_kr,
      dgp.heat_cap_vol, dgp.internal_energy))
    return false;
  return true;
}

gas_pair::gas_pair(gas_t f, gas_t s) {
  if (s > f)
    std::swap(s, f);
  i = f;
  j = s;
}

bool gas_pair::operator<(const gas_pair &rhs) const {
  if (i != rhs.i) {
    return i < rhs.i;
  } else {
    return j < rhs.j;
  }
}

const_dyn_union::~const_dyn_union() {}

calculation_state_log &calculation_state_log::SetDynPars(
    const dyn_parameters &dp) {
  dyn_pars = dp;
  initialized |= (f_vol | f_pres | f_temp);
  if (dp.setup & DYNAMIC_HEAT_CAP_VOL)
    initialized |= f_dcv;
  if (dp.setup & DYNAMIC_HEAT_CAP_PRES)
    initialized |= f_dcp;
  if (dp.setup & DYNAMIC_INTERNAL_ENERGY)
    initialized |= f_din;
  if (dp.setup & DYNAMIC_BETA_KR)
    initialized |= f_dbk;
  if (dp.setup & DYNAMIC_ENTALPHY)
    initialized |= f_enthalpy;
  return *this;
}

calculation_state_log &calculation_state_log::SetCalculationInfo(
    calculation_info *ci) {
  calculation = ci;
  return *this;
}

bool gas_char::IsAromatic(gas_t gas) {
#ifdef ASSIGNMENT_TRACE_COMPONENTS
  return gas_char::is_in(gas, {
      CH(BENZENE), CH(TOLUENE), CH(ETHYLBENZENE), CH(O_XYLENE)});
#else
  return false;
#endif  // ASSIGNMENT_TRACE_COMPONENTS
}

bool gas_char::IsHydrocarbon(gas_t gas) {
  bool is_hc = gas_char::is_in(gas, {CH(METHANE), CH(ETHANE),
      CH(PROPANE), CH(HEXANE), CH(N_BUTANE), CH(ISO_BUTANE),
      CH(N_PENTANE), CH(ISO_PENTANE), CH(HEPTANE),
      CH(OCTANE), CH(NONANE), CH(DECANE)});
#ifdef ASSIGNMENT_TRACE_COMPONENTS
  if (!is_hc) {
    is_hc = gas_char::is_in(gas, {CH(NEO_PENTANE), CH(METHYL_PENTANE2),
         CH(METHYL_PENTANE3), CH(DIMETHYL_BUTANE2), CH(DIMETHYL_BUTANE3)});
  }
#endif  // ASSIGNMENT_TRACE_COMPONENTS
  return is_hc;
}

// Парафины - алканы с формулой C[n]H[2n]
bool gas_char::IsCycleParafine(gas_t gas) {
  bool hc = is_in(gas, {CH(CYCLOPENTENE), CH(MCYCLOPENTENE), CH(ECYCLOPENTENE),
      CH(CYCLOHEXANE), CH(MCYCLOHEXANE), CH(ECYCLOHEXANE)});
  return hc;
}

bool gas_char::is_in(gas_t g, const std::vector<gas_t> &v) {
  for (const auto &x: v)
    if (x == g)
      return true;
  return false;
}
