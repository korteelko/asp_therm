/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#include "gas_ng_gost56851.h"

#include "Logging.h"
#include "gas_ng_gost_defines.h"

#include <assert.h>

namespace {
struct max_valid_limits_t {
  double min, max;
};
/* GOST 56851 */
/**
 * \brief Контейнер допусков молекулярных масс
 *   компонентов газовой смеси для ГОСТ модели
 * */
std::map<gas_t, max_valid_limits_t> mix_valid_molar =
    std::map<gas_t, max_valid_limits_t>{{CH(METHANE), {0.89, 0.99999}},
                                        {CH(ETHANE), {0.0, 0.07}},
                                        {CH(PROPANE), {0.0, 0.02}},
                                        {CH(ALL_BUTANES), {0.0, 0.009}},
                                        {CH(ALL_PENTANES), {0.0, 0.0035}},
                                        {CH(HEXANE), {0.0, 0.001}},
                                        {CH(NITROGEN), {0.0, 0.2}},
                                        {CH(CARBON_DIOXIDE), {0.0, 0.2}},
                                        {CH(HELIUM), {0.0, 0.005}},
                                        {CH(HYDROGEN), {0.0, 0.1}},
                                        // SUM of others
                                        {CH(UNDEFINED), {0.0, 0.0015}}};

}  // namespace

GasParametersGost56851Dyn::GasParametersGost56851Dyn(parameters prs,
                                                     const_parameters cgp,
                                                     ng_gost_mix components)
    : GasParameters(prs, cgp, dyn_parameters()),
      components_(components) {}

// todo: merge this method with GasParametersGost30319Dyn::calcPseudocriticVPT
GasParametersGost56851Dyn::pseudocritic_parameters
    GasParametersGost56851Dyn::calcPseudocriticVPT(ng_gost_mix components) {
  double vol = 0.0;
  double temp = 0.0;
  double press_var = 0.0;
  double tmp_var = 0;
  double Mi = 0.0, Mj = 0.0;
  const component_characteristics* x_ch = nullptr;
  const A2_SPG_coef* bin_coefs = nullptr;
  const critical_params* i_cp = nullptr;
  const critical_params* j_cp = nullptr;
  for (size_t i = 0; i < components.size(); ++i) {
    if ((x_ch = get_characteristics(components[i].first))) {
      Mi = x_ch->M;
    } else {
      Logging::Append(
          "init pseudocritic by gost 56851 model\n"
          "  undefined component: #" +
          std::to_string(components[i].first));
      continue;
    }
    if (!(i_cp = get_critical_params(components[i].first)))
      continue;
    for (size_t j = 0; j < components.size(); ++j) {
      if ((x_ch = get_characteristics(components[j].first))) {
        Mj = x_ch->M;
      } else {
        Logging::Append(
            "init pseudocritic by gost 56851 model\n"
            "  undefined component: #" +
            std::to_string(components[j].first));
        continue;
      }
      bin_coefs = get_A2_SPG_coefs(components[i].first, components[j].first);
      if (!(j_cp = get_critical_params(components[j].first)))
        continue;
      tmp_var = components[i].second * components[j].second
                * pow(pow(Mi / i_cp->density, 0.333333)
                + pow(Mj / j_cp->density, 0.333333), 3.0) * bin_coefs->a;
      vol += tmp_var;
      temp += tmp_var * sqrt(i_cp->temperature * j_cp->temperature)
              * bin_coefs->b;
    }
    press_var += components[i].second * i_cp->acentric;
  }
  press_var *= 0.08;
  press_var = 0.291 - press_var;
  pseudocritic_parameters pseudocrit_vpt;
  pseudocrit_vpt.vpt.volume = 0.125 * vol;
  pseudocrit_vpt.vpt.temperature = 0.125 * temp / vol;
  pseudocrit_vpt.vpt.pressure = 1000 * GAS_CONSTANT
                            * pseudocrit_vpt.vpt.temperature * press_var
                            / pseudocrit_vpt.vpt.volume;
  pseudocrit_vpt.z = press_var;
  return pseudocrit_vpt;
}

GasParametersGost56851Dyn* GasParametersGost56851Dyn::Init(gas_params_input gpi) {
  GasParametersGost56851Dyn*  res = nullptr;
}

void GasParametersGost56851Dyn::csetParameters(double v,
                                               double p,
                                               double t,
                                               state_phase sp) {
  assert(0);
}

double GasParametersGost56851Dyn::cCalculateVolume(double p, double t) {
  assert(0);
}
