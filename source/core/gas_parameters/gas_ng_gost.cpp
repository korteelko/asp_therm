/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#include "gas_ng_gost.h"

#include "atherm_common.h"
#include "gas_ng_gost_defines.h"
#include "Logging.h"

#include <array>
#include <functional>
#include <numeric>
#include <utility>

#include <assert.h>
#include <math.h>


/* Параметры давления и температуры в пределах допустимости
 *  для ГОСТ 30319-2015 */
#define gost_30319_within(p, t) \
    ((p >= 100000 && p <= 30000000) && (t >= 250 && t <= 350))

// ErrorWrap GasParameters_NG_Gost_dyn::init_error;

namespace {
const double Lt = 1.0;
// typedef std::pair<double, double> mix_valid_limits_t;
struct max_valid_limits_t {
  double min,
         max;
};
/* GOST 30319.3-2015, table2, page 11 */
std::map<gas_t, max_valid_limits_t> mix_valid_molar =
    std::map<gas_t, max_valid_limits_t> {
  {CH(METHANE), {0.7, 0.99999}},
  {CH(ETHANE),  {0.0, 0.1}},
  {CH(PROPANE), {0.0, 0.035}},
  {CH(ALL_BUTANES), {0.0, 0.015}},
  {CH(ALL_PENTANES), {0.0, 0.005}},
#ifdef ISO_20765
  {CH(ALL_OTHER_ALKANES), {0.0, 0.0005}},
#endif  // ISO_20765
  {CH(HEXANE),  {0.0, 0.001}},
  {CH(NITROGEN), {0.0, 0.2}},
  {CH(CARBON_DIOXIDE), {0.0, 0.2}},
  {CH(HELIUM), {0.0, 0.005}},
  {CH(HYDROGEN), {0.0, 0.1}},
  // SUM of others
  {CH(UNDEFINED), {0.0, 0.0015}}
};

bool is_valid_limits(
    std::map<const gas_t, max_valid_limits_t>::const_iterator limit_it,
    double part) {
  if ((limit_it->second.min < (part + FLOAT_ACCURACY)) &&
      ((limit_it->second.max + FLOAT_ACCURACY) > part))
    return true;
  return false;
}

bool is_valid_limits(const ng_gost_component &component) {
  const auto limits_it = mix_valid_molar.find(component.first);
  if (limits_it == mix_valid_molar.cend())
    return false;
  return is_valid_limits(limits_it, component.second);
}

// init check
bool is_valid_limits(const ng_gost_mix &components, bool use_iso20675) {
  if (components.empty())
    return false;
  bool is_valid = true;
  ng_gost_component butanes(CH(ALL_BUTANES), 0.0);
  ng_gost_component pentanes(CH(ALL_PENTANES), 0.0);
#ifdef ISO_20765
  ng_gost_component other_alkanes(CH(ALL_OTHER_ALKANES), 0.0);
#endif  // ISO_20765
  ng_gost_component others(CH(UNDEFINED), 0.0);
  // todo: replace with macroces
  for (const auto &component : components) {
    const auto limits_it = mix_valid_molar.find(component.first);
    if (limits_it == mix_valid_molar.cend()) {
      if (component.first == CH(ISO_PENTANE) || component.first == CH(N_PENTANE)) {
        pentanes.second += component.second;
      } else if (component.first == CH(ISO_BUTANE) || component.first == CH(N_BUTANE)) {
        butanes.second += component.second;
    #ifdef ISO_20765
      } else if (component.first == CH(OCTANE) || component.first == CH(NONANE) ||
          component.first == CH(DECANE)) {
        other_alkanes.second += component.second;
    #endif  // ISO_20765
      } else {
        others.second += component.second;
      }
      continue;
    }
    if (is_valid_limits(limits_it, component.second) == false)
      return false;
  }
  is_valid &= is_valid_limits(butanes);
  is_valid &= is_valid_limits(pentanes);
#ifdef ISO_20765
  if (use_iso20675)
    is_valid &= is_valid_limits(other_alkanes);
#endif  // ISO_20765
  is_valid &= is_valid_limits(others);
  return is_valid;
}
}  // anonymus namespace

GasParametersGost30319Dyn *GasParametersGost30319Dyn::Init(
    gas_params_input gpi, bool use_iso) {
  if (gpi.const_dyn.ng_gost_components->empty()) {
    GasParameters::init_error.SetError(ERROR_PAIR_DEFAULT(ERROR_INIT_NULLP_ST));
    return nullptr;
  }
  if (!is_valid_limits(*gpi.const_dyn.ng_gost_components, use_iso)) {
    GasParametersGost30319Dyn::init_error.SetError(ERROR_INIT_T,
        "natural gas model init error:\n components limits check fail\n");
    return nullptr;
  }
  // костыли-костылёчки
  // todo: repair this!!!
  std::unique_ptr<const_parameters> cgp(const_parameters::Init(
      GAS_TYPE_MIX, 1.0, 1.0, 1.0, 1.0, 1.0, 0.1));
  std::unique_ptr<dyn_parameters> dgp(dyn_parameters::Init(
      DYNAMIC_SETUP_DEFAULT, 1.0, 1.0, 1.0, {1.0, 1.0, 1.0}));
  if (cgp == nullptr || dgp == nullptr) {
#ifdef _DEBUG
    assert(0);
#endif  // _DEBUG
    return nullptr;
  }
  return new GasParametersGost30319Dyn({0.0, gpi.p, gpi.t},
      *cgp, *dgp, *gpi.const_dyn.ng_gost_components, use_iso);
}

GasParametersGost30319Dyn::GasParametersGost30319Dyn(parameters prs,
    const_parameters cgp, dyn_parameters dgp, ng_gost_mix components,
    bool use_iso)
  : GasParameters(prs, cgp, dgp), components_(components),
    use_iso20765_(use_iso) {
  if (setFuncCoefficients())
    setStartCondition();
}

bool GasParametersGost30319Dyn::setFuncCoefficients() {
  if (!init_kx()) {
    set_V();
    set_Q();
    set_F();
    set_G();
    set_Bn();
    set_Cn();
    return true;
  }
  return false;
}

void GasParametersGost30319Dyn::setStartCondition() {
  if (set_molar_data()) {
    error_.SetError(ERROR_INIT_T, "udefined component of natural gas");
  } else {
    set_p0m();
    bool is_valid = (set_cp0r() == ERROR_SUCCESS_T);
  #if defined(ISO_20765)
    is_valid &= (use_iso20765_) ? (set_fi0r() == ERROR_SUCCESS_T) : true;
  #endif  // ISO_20765
    if (is_valid) {
      // да, это неправильно и снова отдельная функция должна быть
      update_dynamic();
      if (!init_pseudocrit_vpte()) {
        set_volume();
      } else {
        error_.SetError(ERROR_INIT_T, "undefined component of natural gas\n");
      }
    }
  }
}

// calculating
merror_t GasParametersGost30319Dyn::init_kx() {
  coef_kx_ = 0.0;
  const component_characteristics *xi_ch = nullptr;
  for (size_t i = 0; i < components_.size(); ++i) {
    xi_ch = get_characteristics(components_[i].first);
    if (xi_ch == nullptr)
      return error_.SetError(ERROR_INIT_T, "undefined component in gost model");
    coef_kx_ += components_[i].second * pow(xi_ch->K, 2.5);
  }
  coef_kx_ *= coef_kx_;
  double associate_Kx_part = 0.0;
  const binary_associate_coef *assoc_coefs = nullptr;
  const component_characteristics *xj_ch = nullptr;
  for (size_t i = 0; i < components_.size() - 1; ++i) {
    xi_ch = get_characteristics(components_[i].first);
    for (size_t j = i + 1; j < components_.size(); ++j) {
      xj_ch = get_characteristics(components_[j].first);
      assoc_coefs = get_binary_associate_coefs(
          components_[i].first, components_[j].first);
      associate_Kx_part += components_[i].second * components_[j].second *
          ((pow(assoc_coefs->K, 5.0) - 1.0) * pow(xi_ch->K * xj_ch->K, 2.5));
    }
  }
  associate_Kx_part *= 2.0;
  coef_kx_ += associate_Kx_part;
  coef_kx_ = pow(coef_kx_, 0.2);
  return ERROR_SUCCESS_T;
}

void GasParametersGost30319Dyn::set_V() {
  double V = 0.0;
  const component_characteristics *xi_ch = nullptr;
  for (size_t i = 0; i < components_.size(); ++i) {
    xi_ch = get_characteristics(components_[i].first);
    if (xi_ch == nullptr) {
      error_.SetError(ERROR_INIT_T, "undefined component in gost model");
      return;
    }
    V += components_[i].second * pow(xi_ch->E, 2.5);
  }
  V *= V;
  double associate_V_part = 0.0;
  const binary_associate_coef *assoc_coefs = nullptr;
  const component_characteristics *xj_ch = nullptr;
  for (size_t i = 0; i < components_.size() - 1; ++i) {
    xi_ch = get_characteristics(components_[i].first);
    for (size_t j = i + 1; j < components_.size(); ++j) {
      xj_ch = get_characteristics(components_[j].first);
      assoc_coefs = get_binary_associate_coefs(
          components_[i].first, components_[j].first);
      associate_V_part += components_[i].second * components_[j].second *
          ((pow(assoc_coefs->V, 5.0) - 1.0) * pow(xi_ch->E * xj_ch->E, 2.5));
    }
  }
  associate_V_part *= 2.0;
  V += associate_V_part;
  V = pow(V, 0.2);
  coef_V_ = V;
}

void GasParametersGost30319Dyn::set_Q() {
  coef_Q_ = 0.0;
  const component_characteristics *xi_ch = nullptr;
  for (size_t i = 0; i < components_.size(); ++i) {
    xi_ch = get_characteristics(components_[i].first);
    coef_Q_ += components_[i].second * xi_ch->Q;
  }
}

void GasParametersGost30319Dyn::set_F() {
  coef_F_ = 0.0;
  const component_characteristics *xi_ch = nullptr;
  for (size_t i = 0; i < components_.size(); ++i) {
    xi_ch = get_characteristics(components_[i].first);
    coef_F_ += pow(components_[i].second, 2.0) * xi_ch->F;
  }
}

void GasParametersGost30319Dyn::set_G() {
  double G = 0.0;
  const component_characteristics *xi_ch = nullptr;
  for (size_t i = 0; i < components_.size(); ++i) {
    xi_ch = get_characteristics(components_[i].first);
    if (xi_ch == nullptr) {
      error_.SetError(ERROR_INIT_T, "undefined component in gost model");
      return;
    }
    G += components_[i].second * xi_ch->G;
  }
  double associate_G_part = 0.0;
  const binary_associate_coef *assoc_coefs = nullptr;
  const component_characteristics *xj_ch = nullptr;
  for (size_t i = 0; i < components_.size() - 1; ++i) {
    xi_ch = get_characteristics(components_[i].first);
    for (size_t j = i + 1; j < components_.size(); ++j) {
      xj_ch = get_characteristics(components_[j].first);
      assoc_coefs = get_binary_associate_coefs(
          components_[i].first, components_[j].first);
      associate_G_part += components_[i].second * components_[j].second *
          ((assoc_coefs->G - 1.0) * (xi_ch->G + xj_ch->G));
    }
  }
  G += associate_G_part;
  coef_G_ = G;
}

void GasParametersGost30319Dyn::set_Bn() {
  size_t n_max = A0_3_coefs_count;
  if (!Bn_.empty())
    Bn_.clear();
  Bn_.assign(n_max, 0.0);
  const component_characteristics *xi_ch = nullptr;
  const component_characteristics *xj_ch = nullptr;
  const binary_associate_coef *assoc_coef = nullptr;
  for (size_t n = 0; n < n_max; ++n) {
    // set Bnij
    const A0_3_coef &A3c = A0_3_coefs[n];
    for(size_t i = 0; i < components_.size(); ++i) {
      xi_ch = get_characteristics(components_[i].first);
      for (size_t j = 0; j < components_.size(); ++j) {
        xj_ch = get_characteristics(components_[j].first);
        assoc_coef = get_binary_associate_coefs(
            components_[i].first, components_[j].first);
        double Gij = assoc_coef->G * (xi_ch->G + xj_ch->G) / 2.0;
        double Bnij = pow(Gij + 1.0 - A3c.g, A3c.g) *
            pow(xi_ch->Q*xj_ch->Q + 1.0 - A3c.q, A3c.q) *
            pow(sqrt(xi_ch->F*xj_ch->F) + 1.0 - A3c.f, A3c.f) *
            pow(xi_ch->S*xj_ch->S + 1.0 - A3c.s, A3c.s) *
            pow(xi_ch->W*xj_ch->W + 1.0 - A3c.w, A3c.w);
        double Eij = assoc_coef->E * sqrt(xi_ch->E*xj_ch->E);
        Bn_[n] += components_[i].second * components_[j].second * Bnij *
            pow(Eij, A3c.u) * pow(xi_ch->K * xj_ch->K, 1.5);
      }
    }
  }
}

void GasParametersGost30319Dyn::set_Cn() {
  size_t n_max = A0_3_coefs_count;
  if (!Cn_.empty())
    Cn_.clear();
  Cn_.assign(n_max, 0.0);
  for (size_t n = 0; n < n_max; ++n) {
    const A0_3_coef &A3c = A0_3_coefs[n];
    Cn_[n] = pow(coef_G_ + 1.0 - A3c.g, A3c.g) * pow(coef_Q_ * coef_Q_  + 1.0 -
        A3c.q, A3c.q) * pow(coef_F_ + 1.0 - A3c.f, A3c.f) * pow(coef_V_, A3c.u);
  }
}

merror_t GasParametersGost30319Dyn::set_molar_data() {
  ng_molar_mass_ = 0.0;
  const component_characteristics *xi_ch = nullptr;
  for (size_t i = 0; i < components_.size(); ++i) {
    xi_ch = get_characteristics(components_[i].first);
    if (xi_ch != nullptr) {
      ng_molar_mass_ += components_[i].second * xi_ch->M;
    } else {
      return ERROR_INIT_T;
    }
  }
  Rm = 1000.0 * GAS_CONSTANT / ng_molar_mass_;
  return ERROR_SUCCESS_T;
}

void GasParametersGost30319Dyn::set_p0m() {
  coef_p0m_ = 0.001 * pow(coef_kx_, -3.0) * GAS_CONSTANT * Lt;
}

merror_t GasParametersGost30319Dyn::init_pseudocrit_vpte() {
  double vol = 0.0;
  double temp = 0.0;
  double press_var = 0.0;
  double tmp_var = 0;
  double Mi = 0.0,
         Mj = 0.0;
  const component_characteristics *x_ch = nullptr;
  const critical_params *i_cp = nullptr;
  const critical_params *j_cp = nullptr;
  for (size_t i = 0; i < components_.size(); ++i) {
    if ((x_ch = get_characteristics(components_[i].first))) {
      Mi = x_ch->M;
    } else {
      Logging::Append("init pseudocritic by gost model\n"
          "  undefined component: #" + std::to_string(components_[i].first));
      continue;
    }
    if (!(i_cp = get_critical_params(components_[i].first)))
      continue;
    for (size_t j = 0; j < components_.size(); ++j) {
      if ((x_ch = get_characteristics(components_[j].first))) {
        Mj = x_ch->M;
      } else {
        Logging::Append("init pseudocritic by gost model\n"
            "  undefined component: #" + std::to_string(components_[j].first));
        continue;
      }
      if (!(j_cp = get_critical_params(components_[j].first)))
        continue;
      tmp_var = components_[i].second * components_[j].second * pow( 
          pow(Mi / i_cp->density, 0.333333) + pow(Mj / j_cp->density, 0.333333), 3.0);
      vol += tmp_var;
      temp += tmp_var * sqrt(i_cp->temperature * j_cp->temperature);
    }
    press_var += components_[i].second * i_cp->acentric;
  }
  press_var *= 0.08;
  press_var = 0.291 - press_var;
  pseudocrit_vpte_.volume = 0.125 * vol;
  pseudocrit_vpte_.temperature = 0.125 * temp / vol;
  pseudocrit_vpte_.pressure = 1000 * GAS_CONSTANT *
      pseudocrit_vpte_.temperature * press_var / pseudocrit_vpte_.volume;
  return ERROR_SUCCESS_T;
}

merror_t GasParametersGost30319Dyn::set_cp0r() {
  merror_t error = ERROR_SUCCESS_T;
  double cp0r = 0.0;
  double tet = Lt / vpte_.temperature;
  auto pow_sinh = [tet] (double C, double D) {
    return C * pow(D * tet / sinh(D * tet), 2.0);};
  auto pow_cosh = [tet] (double C, double D) {
    return C * pow(D * tet / cosh(D * tet), 2.0);};
  const A4_coef *cp_coefs = nullptr;
  const component_characteristics *x_ch = nullptr;
  for (size_t i = 0; i < components_.size(); ++i) {
    cp_coefs = get_A4_coefs(components_[i].first);
    x_ch = get_characteristics(components_[i].first);
    if (cp_coefs != nullptr) {
      // by Appex B of ISO 20765
      cp0r += components_[i].second * (cp_coefs->B + pow_sinh(cp_coefs->C, cp_coefs->D) +
          pow_cosh(cp_coefs->E, cp_coefs->F) + pow_sinh(cp_coefs->G, cp_coefs->H) +
          pow_cosh(cp_coefs->I, cp_coefs->J))
          / GAS_CONSTANT * x_ch->M;
    } else {
      error = error_.SetError(ERROR_INIT_T,
          "ГОСТ модель: cp0r, не распознан компонент");
    }
  }
  ng_gost_params_.cp0r = cp0r;
  return error;
}

#if defined(ISO_20765)
merror_t GasParametersGost30319Dyn::set_fi0r() {
  merror_t error = ERROR_SUCCESS_T;
  double fi0r = 0.0,
         fi0r_t = 0.0;
  double tet = Lt / vpte_.temperature,
         tetT = Lt / 298.15;
  double sigm = calculate_sigma(vpte_.pressure, vpte_.temperature),
         sigmT = calculate_sigma(101325.0, 298.15);
  if (is_above0(sigm) && is_above0(sigmT)) {
    double appendix = std::log(tetT / tet) + std::log(sigm / sigmT);
    auto ln_sinh = [tet] (double C, double D) {
      return C * std::log(sinh(D * tet));};
    auto ln_cosh = [tet] (double C, double D) {
      return C * std::log(cosh(D * tet));};
    const A4_coef *cpc = nullptr;
    const component_characteristics *x_ch = nullptr;
    for (size_t i = 0; i < components_.size(); ++i) {
      cpc = get_A4_coefs(components_[i].first);
      x_ch = get_characteristics(components_[i].first);
      if (cpc != nullptr) {
        // by Appex B of ISO 20765
        fi0r += components_[i].second * (cpc->A1 + cpc->A2 * tet +
            cpc->B * std::log(tet) + ln_sinh(cpc->C, cpc->D) -
            ln_cosh(cpc->E, cpc->F) + ln_sinh(cpc->G, cpc->H) -
            ln_cosh(cpc->I, cpc->J) + std::log(components_[i].second)) +
            appendix;
        fi0r_t += components_[i].second * (cpc->A2 + (cpc->B - 1.0) / tet +
            cpc->C * cpc->D * cosh(cpc->D * tet) / sinh(cpc->D * tet) -
            cpc->E * cpc->F * sinh(cpc->F * tet) / cosh(cpc->F * tet) +
            cpc->G * cpc->H * cosh(cpc->H * tet) / sinh(cpc->H * tet) -
            cpc->I * cpc->J * sinh(cpc->J * tet) / cosh(cpc->J * tet));
      } else {
        error = error_.SetError(ERROR_INIT_T,
            "ГОСТ модель: fi0r, не распознан компонент");
        break;
      }
    }
    ng_gost_params_.fi0r = fi0r;
    ng_gost_params_.fi0r_t = fi0r_t;
    ng_gost_params_.fi0r_tt = (1.0 - ng_gost_params_.cp0r / Rm) / (tet * tet);
  } else {
    error = error_.SetError(ERROR_INIT_T, "ГОСТ модель: fi0r, результат расчёта "
        "приведённой плотности некорректен.");
  }
  return error;
}
#endif  // ISO_20765

merror_t GasParametersGost30319Dyn::set_volume() {
  if (check_pt_limits(vpte_.pressure, vpte_.temperature))
    return error_.GetErrorCode();
  double sigma = calculate_sigma(vpte_.pressure, vpte_.temperature);
  /// ? правильно ли
  vpte_.volume = pow(coef_kx_, 3.0) / (ng_molar_mass_ * sigma);
#if defined(ISO_20765)
  if (use_iso20765_) {
    set_iso_params(sigma);
  } else
#endif  // ISO_20765
  {
    set_gost_params(sigma);
  }
  update_dynamic();
  // function end
  return ERROR_SUCCESS_T;
}

void GasParametersGost30319Dyn::set_gost_params(double sigma) {
  ng_gost_params_.A0 = calculate_A0(vpte_.temperature, sigma);
  ng_gost_params_.A1 = calculate_A1(vpte_.temperature, sigma);
  ng_gost_params_.A2 = calculate_A2(vpte_.temperature, sigma);
  ng_gost_params_.A3 = calculate_A3(vpte_.temperature, sigma);
  ng_gost_params_.z = 1.0 + ng_gost_params_.A0;
}

void GasParametersGost30319Dyn::set_viscosity0() {
  double mui = 0.0;
  const A6_coef *coef = nullptr;
  for (auto x : components_) {
    coef = get_A6_coefs(x.first);
    mui = 0.0;
    for (int k = 0; k < 4; ++k)
      mui += coef->k0 * pow(vpte_.temperature / 100.0, k);
  }
  // todo: доделать
  // assert(0);
}

double GasParametersGost30319Dyn::get_Dn(size_t n) const {
  if (n < 12)
    return Bn_[n] * pow(coef_kx_, -3.0);
  else if (n >= 12 && n < 18)
    return Bn_[n] * pow(coef_kx_, -3.0) - Cn_[n];
  return 0.0;
}

double GasParametersGost30319Dyn::get_Un(size_t n) const {
  if (n < 12)
    return 0.0;
  return Cn_[n];
}

#if defined(ISO_20765)
void GasParametersGost30319Dyn::set_iso_params(double sigma) {
  // calculate functions
  set_coefB(vpte_.temperature);
  set_fi(vpte_.temperature, sigma);
  set_fi_der(vpte_.temperature, sigma);

  // set parameters
  ng_gost_params_.z = sigma * ng_gost_params_.fi_d;
}

void GasParametersGost30319Dyn::set_coefB(double t) {
  double tau = 1.0 / t;
  ng_gost_params_.B = 0.0;
  for (size_t n = 0; n < 18; ++n)
    ng_gost_params_.B += Bn_[n] * std::pow(tau, A0_3_coefs[n].u);
}

void GasParametersGost30319Dyn::set_fi(double t, double sigma) {
  double tau = 1.0 / t;
  ng_gost_params_.fi = ng_gost_params_.fi0r +
      ng_gost_params_.B * sigma / std::pow(coef_kx_, 3.0);
  double c1 = 0.0, c2 = 0.0;
  for (size_t n = 12; n < 18; ++n)
    c1 += Cn_[n] * std::pow(tau, A0_3_coefs[n].u);
  c1 *= sigma;
  for (size_t n = 12; n < 58; ++n)
    c2 += Cn_[n] * std::pow(tau, A0_3_coefs[n].u) *
        std::pow(sigma, A0_3_coefs[n].b) *
        std::exp(-A0_3_coefs[n].c * std::pow(sigma, A0_3_coefs[n].k));
  ng_gost_params_.fi += -c1 + c2;
}

/* наверное излишне оптимизировано */
void GasParametersGost30319Dyn::set_fi_der(double t, double sigma) {
  double tau = 1.0 / t;
  double s_k = sigma / pow(coef_kx_, 3.0);
  double fi_t = tau * ng_gost_params_.fi0r_t,
         fi_tt = tau * tau * ng_gost_params_.fi0r_tt,
         fi_d = 1.0 + ng_gost_params_.B * s_k,
         fi_1 = 1.0 + 2.0 * ng_gost_params_.B * s_k,
         fi_2 = 1.0;
  double dfi_t = 0.0, dfi_tt = 0.0, dfi_d = 0.0,
         dfi_1 = 0.0, dfi_2 = 0.0;

  // 1 - 18
  for (size_t n = 0; n < 18; ++n) {
    const A0_3_coef &A3c = A0_3_coefs[n];
    double d1 = A3c.u * Bn_[n] * pow(tau, A3c.u);
    double d2 = (A3c.u - 1.0) * d1;
    dfi_t += d1;
    dfi_tt += d2;
    dfi_2 += d2 / A3c.u;
  }
  fi_t += s_k * dfi_t;
  fi_tt += s_k * dfi_tt;
  fi_2 += s_k * dfi_2;

  // 13 - 18
  dfi_t = 0.0, dfi_tt = 0.0, dfi_d = 0.0, dfi_1 = 0.0, dfi_2 = 0.0;
  for (size_t n = 12; n < 18; ++n) {
    const A0_3_coef &A3c = A0_3_coefs[n];
    double d1 = A3c.u * Cn_[n] * pow(tau, A3c.u);
    double d2 = (A3c.u - 1.0) * d1;
    double d3 = d1 / A3c.u;
    dfi_t += d1;
    dfi_tt += d2;
    dfi_d += d3;
    dfi_1 += d3;
    dfi_2 += d2 / A3c.u;
  }
  fi_t += -sigma * dfi_t;
  fi_tt += -sigma * dfi_tt;
  fi_d += -sigma * dfi_d;
  fi_1 += -2.0 * sigma * dfi_1;
  fi_2 += -sigma * dfi_2;

  // 13 - 58
  dfi_t = 0.0, dfi_tt = 0.0, dfi_d = 0.0, dfi_1 = 0.0, dfi_2 = 0.0;
  for (size_t n = 12; n < 58; ++n) {
    const A0_3_coef &A3c = A0_3_coefs[n];
    double d1 = A3c.u * Cn_[n] * pow(tau, A3c.u) * pow(sigma, A3c.b) *
        exp(-A3c.c * pow(sigma, A3c.k));
    double d2 = (A3c.u - 1.0) * d1;
    double k3 = A3c.b - A3c.c * A3c.k * pow(sigma, A3c.k);
    double d3 = d1 * k3 / A3c.u;
    double d4 = d1 * (k3 - A3c.k * A3c.c * A3c.k * pow(sigma, A3c.k) +
        pow(k3, 2.0)) / A3c.u;
    double d5 = d3 * (1.0 - A3c.u);
    dfi_t += d1;
    dfi_tt += d2;
    dfi_d += d3;
    dfi_1 += d4;
    dfi_2 += d5;
  }
  fi_t += dfi_t;
  fi_tt += dfi_tt;
  fi_d += dfi_d;
  fi_1 += dfi_1;
  fi_2 += dfi_2;

  ng_gost_params_.fi_t = fi_t / tau;
  ng_gost_params_.fi_tt = fi_tt / tau / tau;
  ng_gost_params_.fi_d = fi_t / sigma;
  ng_gost_params_.fi_1 = fi_1;
  ng_gost_params_.fi_2 = fi_2;
}
#endif  // ISO_20765

double GasParametersGost30319Dyn::calculate_sigma(double p, double t) {
  double sigm = sigma_start(p, t),
         tau = t / Lt;
  double A0 = calculate_A0(t, sigm);
  // todo: remove magic numbers!!!
  double pi_calc = sigm * tau * (1.0 + A0),
         pi = 0.000001 * p / coef_p0m_;
  int loop_max = 3000;
  while (--loop_max > 0) {
    if (abs(pi_calc - pi) / pi < 0.000001)
      break;
    sigm += calculate_d_sigm(t, p, sigm);
    A0 = calculate_A0(t, sigm);
    pi_calc = sigm * tau * (1.0 + A0);
  }
#ifdef _DEBUG
  if (loop_max == 0)
    assert(0 && "so many roads but assert");
#else
  assert(0);
  // set error, break procedure
#endif  // _DEBUG
  return sigm;
}

void GasParametersGost30319Dyn::update_dynamic() {
#if defined(ISO_20765)
  if (use_iso20765_) {
    double tau = Lt / vpte_.temperature;
    ng_gost_params_.u = ng_gost_params_.fi_t * Rm;
    ng_gost_params_.h = (ng_gost_params_.u + ng_gost_params_.z) * Rm * vpte_.temperature;
    ng_gost_params_.s = (tau * ng_gost_params_.fi_t - ng_gost_params_.fi) * Rm;
    ng_gost_params_.cv = - tau * tau * ng_gost_params_.fi_tt * Rm;
    ng_gost_params_.cp = ng_gost_params_.cv + ng_gost_params_.fi_2 *
        ng_gost_params_.fi_2 * Rm / ng_gost_params_.fi_1;
    ng_gost_params_.k = ng_gost_params_.fi_1 * ng_gost_params_.cp /
        (ng_gost_params_.z * ng_gost_params_.cv);
    ng_gost_params_.w = sqrt(ng_gost_params_.fi_1 * ng_gost_params_.cp *
        vpte_.temperature * Rm / (ng_molar_mass_ * ng_gost_params_.cv));
    // todo: add another one
    std::map<dyn_setup, double> params = {
      {DYNAMIC_HEAT_CAP_PRES, ng_gost_params_.cp},
      {DYNAMIC_HEAT_CAP_VOL, ng_gost_params_.cv},
      {DYNAMIC_INTERNAL_ENERGY, ng_gost_params_.u}
    };
    dyn_params_.ResetParameters(vpte_, params);
  } else
#endif  // ISO_20765
  {
    auto t = 1.0 + ng_gost_params_.A1 + std::pow(1.0 + ng_gost_params_.A2, 2.0) /
        (ng_gost_params_.cp0r - 1.0 + ng_gost_params_.A3);
    ng_gost_params_.k = t / ng_gost_params_.z;
    ng_gost_params_.w = sqrt(Rm * t * vpte_.temperature );
    dyn_params_.ResetParameters(0, 0.0, 0.0, 0.0, vpte_);
  }
}

merror_t GasParametersGost30319Dyn::check_pt_limits(double p, double t) {
  /* ckeck pressure[0.1, 30.0]MPa, temperature[250,350]K */
  return gost_30319_within(p, t) ?
      ERROR_SUCCESS_T : error_.SetError(
      ERROR_CALCULATE_T, "check ng_gost limits");
}

/* check 07_11_19 */
double GasParametersGost30319Dyn::sigma_start(double p, double t) const {
  return 0.001 * p * pow(coef_kx_, 3.0) / (GAS_CONSTANT * t);
}

/* check 07_11_19 */
double GasParametersGost30319Dyn::calculate_d_sigm(
    double t, double p, double sigm) const {
  double tau = t / Lt,
         pi  = 0.000001 * p / coef_p0m_,
         d_sigm = (pi / tau - (1.0 + calculate_A0(t, sigm))*sigm) /
             (1.0 + calculate_A1(t, sigm));
  return d_sigm;
}

//   dens is sigma, temp is tau
/* check 07_11_19 */
double GasParametersGost30319Dyn::calculate_A0(double t, double sigm) const {
  double A0 = 0.0;
  double tau = t / Lt;
  for (size_t n = 0; n < A0_3_coefs_count; ++n) {
    const A0_3_coef &A3c = A0_3_coefs[n];
    A0 += A3c.a * pow(sigm, A3c.b) * pow(tau, -A3c.u) *
        (A3c.b*get_Dn(n) + (A3c.b - A3c.c*A3c.k*pow(sigm, A3c.k)) * 
        get_Un(n) * exp(-A3c.c * pow(sigm, A3c.k)));
  }
  return A0;
}

/* check 07_11_19 */
double GasParametersGost30319Dyn::calculate_A1(double t, double sigm) const {
  double A1 = 0.0;
  double tau = t / Lt;
  for (size_t n = 0; n < A0_3_coefs_count; ++n) {
    const A0_3_coef &A3c = A0_3_coefs[n];
    A1 += A3c.a * pow(sigm, A3c.b) * pow(tau, -A3c.u) *
        ((A3c.b + 1.0) * A3c.b * get_Dn(n) +
        ((A3c.b - A3c.c * A3c.k * pow(sigm, A3c.k)) *
        (A3c.b - A3c.c * A3c.k * pow(sigm, A3c.k) + 1.0) - 
        A3c.c * A3c.k * A3c.k * pow(sigm, A3c.k)) *
        get_Un(n) * exp(-A3c.c * pow(sigm, A3c.k)));
  }
  return A1;
}

/* check 07_11_19 */
double GasParametersGost30319Dyn::calculate_A2(double t, double sigm) const {
  double A2 = 0.0;
  double tau = t / Lt;
  for (size_t n = 0; n < A0_3_coefs_count; ++n) {
    const A0_3_coef &A3c = A0_3_coefs[n];
    A2 += A3c.a * pow(sigm, A3c.b) * pow(tau, -A3c.u) * (1.0 - A3c.u) *
        (A3c.b*get_Dn(n) + (A3c.b - A3c.c*A3c.k*pow(sigm, A3c.k)) * 
            get_Un(n) * exp(-A3c.c * pow(sigm, A3c.k)));
  }
  return A2;
}

/* check 07_11_19 */
double GasParametersGost30319Dyn::calculate_A3(double t, double sigm) const {
  double A3 = 0.0;
  double tau = t / Lt;
  for (size_t n = 0; n < A0_3_coefs_count; ++n) {
    const A0_3_coef &A3c = A0_3_coefs[n];
    A3 += A3c.a * pow(sigm, A3c.b) * pow(tau, -A3c.u) * (1.0 - A3c.u) * A3c.u *
        (get_Dn(n) + get_Un(n) * exp(-A3c.c * pow(sigm, A3c.k)));
  }
  return A3;
}

void GasParametersGost30319Dyn::csetParameters(
    double v, double p, double t, state_phase) {
  (void)v;
  vpte_.pressure = p;
  vpte_.temperature = t;
  set_volume();
}

double GasParametersGost30319Dyn::cCalculateVolume(double p, double t) {
  parameters bpars = vpte_;
  csetParameters(0.0, p, t, state_phase::GAS);
  double v = vpte_.volume;
  csetParameters(0.0, bpars.pressure, bpars.volume, state_phase::GAS);
  return v;
}

bool GasParametersGost30319Dyn::IsValid() {
  return (error_.GetErrorCode()) ? false : true;
}

bool GasParametersGost30319Dyn::IsValid(parameters prs) {
  return gost_30319_within(prs.pressure, prs.temperature);
}
