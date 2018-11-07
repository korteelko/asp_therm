#include "gas_ng_gost_init.h"

#include "gas_ng_gost_defines.h"
#include "models_errors.h"

#include <utility>

#include <assert.h>
#include <math.h>

namespace {
// typedef std::pair<double, double> mix_valid_limits_t;
struct max_valid_limits_t {
  double min,
         max;
};
std::map<gas_t, max_valid_limits_t> mix_valid_molar =
    std::map<gas_t, max_valid_limits_t> {
  {GAS_TYPE_METHANE, {0.7, 0.99999}},
  {GAS_TYPE_ETHANE,  {0.0, 0.1}},
  {GAS_TYPE_PROPANE, {0.0, 0.035}},
  {GAS_TYPE_ALL_BUTANES,   {0.0, 0.015}},
  {GAS_TYPE_ALL_PENTANES, {0.0, 0.005}},
  {GAS_TYPE_HEXANE,  {0.0, 0.001}},
  {GAS_TYPE_NITROGEN, {0.0, 0.2}},
  {GAS_TYPE_CARBON_DIOXIDE, {0.0, 0.2}},
  {GAS_TYPE_HELIUM, {0.0, 0.005}},
  {GAS_TYPE_HYDROGEN, {0.0, 0.1}},
  // SUM of others
  {GAS_TYPE_UNDEFINED, {0.0, 0.0015}}
};

bool is_valid_limits(std::map<const gas_t, max_valid_limits_t>::const_iterator limit_it,
    double part) {
  if (limit_it->second.min < part && limit_it->second.max > part)
    return true;
  return false;
}

bool is_valid_limits(const ng_gost_component component) {
  const auto limits_it = mix_valid_molar.find(component.first);
  if (limits_it == mix_valid_molar.cend())
    return false;
  return is_valid_limits(limits_it, component.second);
}

// init check
bool is_valid_limits(const ng_gost_mix &components) {
  if (components.empty())
    return false;
  bool is_valid = true;
  ng_gost_component butanes(GAS_TYPE_ALL_BUTANES, 0.0);
  ng_gost_component pentanes(GAS_TYPE_ALL_PENTANES, 0.0);
  ng_gost_component others(GAS_TYPE_UNDEFINED, 0.0);
  for (const auto &component : components) {
    const auto limits_it = mix_valid_molar.find(component.first);
    if (limits_it == mix_valid_molar.cend()) {
      if (component.first == GAS_TYPE_I_PENTANE || component.first == GAS_TYPE_N_PENTANE)
        pentanes.second += component.second;
      else if (component.first == GAS_TYPE_I_BUTANE || component.first == GAS_TYPE_N_BUTANE)
        butanes.second += component.second;
      else
        others.second += component.second;
      continue;
    }
    if (is_valid_limits(limits_it, component.second) == false)
      return false;
  }
  is_valid &= is_valid_limits(butanes);
  is_valid &= is_valid_limits(pentanes);
  is_valid &= is_valid_limits(others);
  return is_valid;
}
}  // anonymus namespace

GasParameters_NG_Gost_dyn::GasParameters_NG_Gost_dyn(
    parameters prs, ng_gost_mix components)
  : components_(components) {
  initKx();
}

GasParameters_NG_Gost_dyn *GasParameters_NG_Gost_dyn::Init(
    gas_params_input &gpi) {
  if (gpi.const_dyn.ng_gost_components->empty()) {
    set_error_code(ERR_INIT_T | ERR_INIT_NULLP_ST | ERR_GAS_MIX);
    return nullptr;
  }
  if (!is_valid_limits(*gpi.const_dyn.ng_gost_components)) {
    return nullptr;
  }
  return new GasParameters_NG_Gost_dyn({0.0, gpi.p, gpi.t},
      *gpi.const_dyn.ng_gost_components);
}

// calculating
void GasParameters_NG_Gost_dyn::initKx() {
  double x = 0.0;
  coef_kx = 0.0;
  for (int i = 0; i < components_.size(); ++i) {

  }
  assert(0);
}
//   dens is sigma, temp is tau
double GasParameters_NG_Gost_dyn::getA0(double dens, double temp) {
  double A0 = 0.0;
  double Un, Dn;
  for (int i = 0; i < A0_3_coefs_count; ++i) {
    const A0_3_coef cfs = A0_3_coefs[i];
    A0 += cfs.a * pow(dens, cfs.b) * pow(temp, -cfs.u) *
        (cfs.b*Dn + (cfs.b - cfs.c*cfs.k*pow(dens, cfs.k)) * 
            Un * exp(-cfs.c * pow(dens, cfs.k)));
  }
  return A0;
}

void GasParameters_NG_Gost_dyn::csetParameters(
    double v, double p, double t, state_phase) {
  assert(0);
}