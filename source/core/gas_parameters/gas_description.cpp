/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020-2021 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#include "gas_description.h"

#include "Common.h"
#include "Logging.h"
#include "models_math.h"

#include <limits>
#include <map>

/**
 * \brief список газов, для которых прописаны файлы параметров,
 *   ключ соответствует имени xml файла
 *   (и строке в БД если мне будет не лень)
 * \todo ето надо объединить с вектором `valid_gases`
 * */
static std::map<std::string, gas_t> gas_names = std::map<std::string, gas_t>{
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
    {"octane", GAS_TYPE_OCTANE},
    {"nonane", GAS_TYPE_NONANE},
    {"decane", GAS_TYPE_DECANE},
    {"carbon_monoxide", GAS_TYPE_CARBON_MONOXIDE},
    {"water", GAS_TYPE_WATER}};

static gas_t valid_gases[] = {
    GAS_TYPE_METHANE,        GAS_TYPE_ETHANE,
    GAS_TYPE_PROPANE,        GAS_TYPE_HYDROGEN_SULFIDE,
    GAS_TYPE_HEXANE,         GAS_TYPE_NITROGEN,
    GAS_TYPE_CARBON_DIOXIDE, GAS_TYPE_HELIUM,
    GAS_TYPE_HYDROGEN,       GAS_TYPE_N_BUTANE,
    GAS_TYPE_ISO_BUTANE,     GAS_TYPE_N_PENTANE,
    GAS_TYPE_ISO_PENTANE,    GAS_TYPE_OXYGEN,
    GAS_TYPE_ARGON,          GAS_TYPE_HEPTANE,
    GAS_TYPE_OCTANE,         GAS_TYPE_NONANE,
    GAS_TYPE_DECANE,         GAS_TYPE_CARBON_MONOXIDE,
    GAS_TYPE_WATER,
#if defined(GASMIX_TEST) && defined(ISO_20765)
    GAS_TYPE_TOLUENE,
#endif  // GASMIX_TEST
    GAS_TYPE_UNDEFINED,      GAS_TYPE_MIX};

static bool is_valid_gas(gas_t gas_name) {
  size_t valid_gases_count = sizeof(valid_gases) / sizeof(*valid_gases);
  for (size_t i = 0; i < valid_gases_count; ++i)
    if (gas_name == valid_gases[i])
      return true;
  return false;
}

gas_t gas_by_name(const std::string& name) {
  return (gas_names.find(name) != gas_names.end()) ? gas_names[name]
                                                   : GAS_TYPE_UNDEFINED;
}

// dyn_parameters
dyn_parameters::dyn_parameters(dyn_setup setup,
                               double cv,
                               double cp,
                               double int_eng,
                               parameters pm)
    : setup(setup),
      heat_cap_vol(cv),
      heat_cap_pres(cp),
      internal_energy(int_eng),
      beta_kr(0.0),
      parm(pm) {
  status = STATUS_OK;
  Update();
}

void dyn_parameters::check_setup() {
  if (setup & DYNAMIC_HEAT_CAP_VOL & DYNAMIC_HEAT_CAP_PRES)
    setup |= DYNAMIC_BETA_KR;
}

merror_t dyn_parameters::check_input(parameters pm) {
  // pm.volume мы не проверяем, т.к. можно пересчитать
  bool correct_input = is_above0(pm.pressure, pm.temperature);
  return (correct_input) ? ERROR_SUCCESS_T : ERROR_INIT_T;
}

dyn_parameters* dyn_parameters::Init(dyn_setup setup,
                                     double cv,
                                     double cp,
                                     double int_eng,
                                     parameters pm) {
  if (dyn_parameters::check_input(pm))
    return nullptr;
  return new dyn_parameters(setup, cv, cp, int_eng, pm);
}

dyn_parameters::dyn_parameters()
    : status(STATUS_NOT),
      setup(0),
      heat_cap_vol(0.0),
      heat_cap_pres(0.0),
      internal_energy(0.0),
      enthalpy(0.0),
      adiabatic(0.0),
      beta_kr(0.0),
      entropy(0.0) {}

merror_t dyn_parameters::ResetParameters(
    parameters pm,
    const std::map<dyn_setup, double>& params) {
  status = STATUS_DEFAULT;
  merror_t error = dyn_parameters::check_input(pm);
  parm = pm;
  if (!error) {
    status = STATUS_OK;
    setup = 0x0;
    for (const auto& x : params) {
      switch (x.first) {
        case DYNAMIC_HEAT_CAP_PRES:
          heat_cap_pres = x.second;
          setup |= x.first;
          break;
        case DYNAMIC_HEAT_CAP_VOL:
          heat_cap_vol = x.second;
          setup |= x.first;
          break;
        case DYNAMIC_INTERNAL_ENERGY:
          internal_energy = x.second;
          setup |= x.first;
          break;
        case DYNAMIC_ENTALPHY:
          enthalpy = x.second;
          setup |= x.first;
          break;
        case DYNAMIC_ADIABATIC:
          adiabatic = x.second;
          setup |= x.first;
          break;
        case DYNAMIC_BETA_KR:
          beta_kr = x.second;
          setup |= x.first;
          break;
        case DYNAMIC_ENTROPY:
          entropy = x.second;
          setup |= x.first;
          break;
      }
    }
  }
  return error;
}

// update beta critical
void dyn_parameters::Update() {
  if (is_status_ok(status)) {
    if ((setup & DYNAMIC_BETA_KR) && (setup & DYNAMIC_ADIABATIC)) {
      beta_kr =
          std::pow(2.0 / (adiabatic + 1.0), adiabatic / (adiabatic - 1.0));
    }
  }
}

const_parameters::const_parameters(gas_t gas_name,
                                   double vk,
                                   double pk,
                                   double tk,
                                   double zk,
                                   double mol,
                                   double R,
                                   double af)
    : gas_name(gas_name),
      critical({.volume = vk, .pressure = pk, .temperature = tk}),
      Z_K(zk),
      acentricfactor(af),
      mp({.mass = mol, .Rm = R}) {}

const_parameters* const_parameters::Init(gas_t gas_name,
                                         double vk,
                                         double pk,
                                         double tk,
                                         double zk,
                                         double mol,
                                         double af) {
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
      return new const_parameters(gas_name, vk, pk, tk, zk, mol,
                                  1000.0 * GAS_CONSTANT / mol, af);
    Logging::Append(ERROR_INIT_T, "const_parameters: invalid gas_name");
  } else {
    Logging::Append(ERROR_INIT_T, "const_parameters: input pars < 0");
  }
  return nullptr;
}

const_parameters::const_parameters(parameters pc,
                                   double zk,
                                   molar_parameters mp)
    : gas_name(GAS_TYPE_MIX),
      critical(pc),
      Z_K(zk),
      acentricfactor(std::numeric_limits<double>::infinity()),
      mp(mp) {
  if (!check_params(pc.volume, pc.pressure, pc.temperature, Z_K, mp.mass))
    throw gparameters_exception(ERROR_INIT_T,
                                "Ошибка инициализации неизменяемых параметров "
                                "смеси. Для доп. информации см. лог файл "
                                    + std::string(Logging::GetLogFile()));
}

//#define

// const_parameters
bool const_parameters::check_params(double vk,
                                    double pk,
                                    double tk,
                                    double zk,
                                    double mol) {
  bool is_valid = true;
  is_valid = is_above0(pk, tk, mol);
  if (is_valid) {
    if (!is_above0(vk)) {
      if (!is_above0(zk)) {
        is_valid = false;
      }
    }
  }
  return is_valid;
}

const_parameters::const_parameters(const const_parameters& cgp)
    : gas_name(cgp.gas_name),
      critical(cgp.critical),
      Z_K(cgp.Z_K),
      acentricfactor(cgp.acentricfactor),
      mp(cgp.mp) {}

bool const_parameters::IsGasmix() const {
  return gas_name == GAS_TYPE_MIX;
}

bool const_parameters::IsAbstractGas() const {
  return gas_name == GAS_TYPE_UNDEFINED;
}

std::string const_parameters::GetString(std::string pref) const {
  std::stringstream ss;
  ss << pref + "Параметры критической точки:\n";
  ss << pref << "p = " << critical.pressure << ", v = " << critical.volume
     << ", t = " << critical.temperature;

  return ss.str();
}

bool is_valid_cgp(const const_parameters& cgp) {
  if (cgp.gas_name != GAS_TYPE_MIX) {
    if (!const_parameters::check_params(
            cgp.critical.volume, cgp.critical.pressure,
            cgp.critical.temperature, cgp.Z_K, cgp.mp.mass,
            // TODO: why Rm was checked twice?
            cgp.mp.Rm)
        || !is_above0(cgp.mp.Rm))
      return false;
  }
  return is_valid_gas(cgp.gas_name) ? true : false;
}

bool is_valid_dgp(const dyn_parameters& dgp) {
  if (!is_above0(dgp.heat_cap_pres, dgp.beta_kr, dgp.heat_cap_vol,
                 dgp.internal_energy))
    return false;
  return true;
}

gparameters_exception::gparameters_exception(const std::string& msg)
    : gparameters_exception(ERROR_GENERAL_T, msg) {}

gparameters_exception::gparameters_exception(merror_t error,
                                             const std::string& msg)
    : error_(error) {
  msg_ = "gparameters_exception";
  if (error_)
    msg_ += "with code of error: " + hex2str(error_);
  msg_ += ":\n\t";
  msg_ += msg;
}

const char* gparameters_exception::what() const noexcept {
  return msg_.c_str();
}

gas_pair::gas_pair(gas_t f, gas_t s) {
  if (s > f)
    std::swap(s, f);
  i = f;
  j = s;
}

bool gas_pair::operator<(const gas_pair& rhs) const {
  if (i != rhs.i) {
    return i < rhs.i;
  } else {
    return j < rhs.j;
  }
}

const_dyn_union::~const_dyn_union() {}

calculation_state_log& calculation_state_log::SetDynPars(
    const dyn_parameters& dp) {
  dyn_pars = dp;
  initialized |= (f_vol | f_pres | f_temp);
  if (dp.setup & DYNAMIC_HEAT_CAP_VOL)
    initialized |= f_dcv;
  if (dp.setup & DYNAMIC_HEAT_CAP_PRES)
    initialized |= f_dcp;
  if (dp.setup & DYNAMIC_INTERNAL_ENERGY)
    initialized |= f_din;
  if (dp.setup & DYNAMIC_ENTALPHY)
    initialized |= f_denthalpy;
  if (dp.setup & DYNAMIC_ADIABATIC)
    initialized |= f_dadiabatic;
  if (dp.setup & DYNAMIC_BETA_KR)
    initialized |= f_dbk;
  if (dp.setup & DYNAMIC_ENTROPY)
    initialized |= f_dentropy;
  return *this;
}

calculation_state_log& calculation_state_log::SetCalculationInfo(
    calculation_info* ci) {
  calculation = ci;
  return *this;
}

bool gas_char::IsAromatic(gas_t gas) {
#ifdef ASSIGNMENT_TRACE_COMPONENTS
  return gas_char::is_in(
      gas, {CH(BENZENE), CH(TOLUENE), CH(ETHYLBENZENE), CH(O_XYLENE)});
#else
  return false;
#endif  // ASSIGNMENT_TRACE_COMPONENTS
}

bool gas_char::IsHydrocarbon(gas_t gas) {
  bool is_hc = gas_char::is_in(
      gas, {CH(METHANE), CH(ETHANE), CH(PROPANE), CH(HEXANE), CH(N_BUTANE),
            CH(ISO_BUTANE), CH(N_PENTANE), CH(ISO_PENTANE), CH(HEPTANE),
            CH(OCTANE), CH(NONANE), CH(DECANE)});
#ifdef ASSIGNMENT_TRACE_COMPONENTS
  if (!is_hc) {
    is_hc = gas_char::is_in(
        gas, {CH(NEO_PENTANE), CH(METHYL_PENTANE2), CH(METHYL_PENTANE3),
              CH(DIMETHYL_BUTANE2), CH(DIMETHYL_BUTANE3)});
  }
#endif  // ASSIGNMENT_TRACE_COMPONENTS
  return is_hc;
}

// Парафины - алканы с формулой C[n]H[2n]
bool gas_char::IsCycleParafine(gas_t gas) {
#ifdef ASSIGNMENT_TRACE_COMPONENTS
  return is_in(gas, {CH(CYCLOPENTENE), CH(MCYCLOPENTENE), CH(ECYCLOPENTENE),
                     CH(CYCLOHEXANE), CH(MCYCLOHEXANE), CH(ECYCLOHEXANE)});
#else
  return false;
#endif
}

bool gas_char::IsNoble(gas_t gas) {
  return is_in(gas, {CH(HELIUM), CH(ARGON)
#ifdef ASSIGNMENT_TRACE_COMPONENTS
                                     ,
                     CH(NEON), CH(KRYPTONE), CH(XENON)
#endif  // ASSIGNMENT_TRACE_COMPONENTS
                    });
}

bool gas_char::is_in(gas_t g, const std::vector<gas_t>& v) {
  for (const auto& x : v)
    if (x == g)
      return true;
  return false;
}
