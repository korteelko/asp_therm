#include "gas_ng_gost.h"

#include "common.h"
#include "gas_ng_gost_defines.h"
#include "models_errors.h"

#include <array>
#include <functional>
#include <utility>

#include <assert.h>
#include <math.h>

namespace {
const double R  = 8.31451;
const double Lt = 1.0;
// typedef std::pair<double, double> mix_valid_limits_t;
struct max_valid_limits_t {
  double min,
         max;
};
/* GOST 30319.3-2015, table2, page 11 */
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
    parameters prs, const_parameters cgp, dyn_parameters dgp, 
    ng_gost_mix components)
  : GasParameters(prs, cgp, dgp), error_(ERR_SUCCESS_T),
    components_(components) {
  if (init_kx())
    return;
  set_V();
  set_Q();
  set_F();
  set_G();
  set_Bn();
  set_Cn();
  if (set_molar_mass()) {
    error_ = set_error_message(
        ERR_INIT_T, "udefined component of natural gas");
    return;
  }
  set_p0m();
  if (init_pseudocrit_vpte()) {
    error_ = set_error_message(
        ERR_INIT_T, "udefined component of natural gas");
    return;
  }
}

GasParameters_NG_Gost_dyn *GasParameters_NG_Gost_dyn::Init(
    gas_params_input gpi) {
  if (gpi.const_dyn.ng_gost_components->empty()) {
    set_error_code(ERR_INIT_T | ERR_INIT_NULLP_ST | ERR_GAS_MIX);
    return nullptr;
  }
  if (!is_valid_limits(*gpi.const_dyn.ng_gost_components)) {
    return nullptr;
  }
  // костыли-костылёчки
  const_parameters *cgp = const_parameters::Init(
      GAS_TYPE_MIX, 1.0, 1.0, 1.0, 1.0, 0.1);
  dyn_parameters *dgp = dyn_parameters::Init(
      1.0, 1.0, 1.0, {1.0, 1.0, 1.0});
  if (cgp == nullptr || dgp == nullptr) {
#ifdef _DEBUG
    assert(0);
#endif  // _DEBUG
    return nullptr;
  }
  return new GasParameters_NG_Gost_dyn({0.0, gpi.p, gpi.t},
      *cgp, *dgp, *gpi.const_dyn.ng_gost_components);
}

void GasParameters_NG_Gost_dyn::set_V() {
  double V = 0.0;
  const component_characteristics *xi_ch = nullptr;
  for (size_t i = 0; i < components_.size(); ++i) {
    xi_ch = get_characteristics(components_[i].first);
    if (xi_ch == nullptr) {
      error_ = set_error_message(
          ERR_INIT_T, "undefined component in gost model");
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

void GasParameters_NG_Gost_dyn::set_Q() {
  coef_Q_ = 0.0;
  const component_characteristics *xi_ch = nullptr;
  for (size_t i = 0; i < components_.size(); ++i) {
    xi_ch = get_characteristics(components_[i].first);
    coef_Q_ += components_[i].second * xi_ch->Q;
  }
}

void GasParameters_NG_Gost_dyn::set_F() {
  coef_F_ = 0.0;
  const component_characteristics *xi_ch = nullptr;
  for (size_t i = 0; i < components_.size(); ++i) {
    xi_ch = get_characteristics(components_[i].first);
    coef_F_ += pow(components_[i].second, 2.0) * xi_ch->F;
  }
}

void GasParameters_NG_Gost_dyn::set_G() {
  double G = 0.0;
  const component_characteristics *xi_ch = nullptr;
  for (size_t i = 0; i < components_.size(); ++i) {
    xi_ch = get_characteristics(components_[i].first);
    if (xi_ch == nullptr) {
      error_ = set_error_message(
          ERR_INIT_T, "undefined component in gost model");
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

void GasParameters_NG_Gost_dyn::set_Bn() {
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
        double Bnij = pow(Gij + 1.0 - A3c.g, A3c.g) * pow(xi_ch->Q*xj_ch->Q + 1.0 - A3c.q, A3c.q) *
            pow(sqrt(xi_ch->F*xj_ch->F) + 1.0 - A3c.f, A3c.f) * pow(xi_ch->S*xj_ch->S + 1.0 - A3c.s, A3c.s) *
            pow(xi_ch->W*xj_ch->W + 1.0 - A3c.w, A3c.w);
        double Eij = assoc_coef->E * sqrt(xi_ch->E*xj_ch->E);
        Bn_[n] += components_[i].second * components_[j].second * Bnij * pow(Eij, A3c.u) *
            pow(xi_ch->K * xj_ch->K, 1.5);
      }
    }
  }
}

void GasParameters_NG_Gost_dyn::set_Cn() {
  size_t n_max = A0_3_coefs_count;
  if (!Cn_.empty())
    Cn_.clear();
  Cn_.assign(n_max, 0.0);
  for (size_t n = 0; n < n_max; ++n) {
    const A0_3_coef &A3c = A0_3_coefs[n];
    Cn_[n] = pow(coef_G_ +1.0 - A3c.g, A3c.g) * pow(coef_Q_ * coef_Q_  + 1.0 - A3c.q, A3c.q) *
        pow(coef_F_ + 1.0 - A3c.f, A3c.f) * pow(coef_V_, A3c.u);
  }
}

merror_t GasParameters_NG_Gost_dyn::set_molar_mass() {
  ng_molar_mass_ = 0.0;
  const component_characteristics *xi_ch = nullptr;
  const A9_molar_mass *m_mas = nullptr;
  for (size_t i = 0; i < components_.size(); ++i) {
    xi_ch = get_characteristics(components_[i].first);
    if (xi_ch != nullptr) {
      ng_molar_mass_ += components_[i].second * xi_ch->M;
    } else {
      m_mas = get_molar_mass(components_[i].first);
      if (m_mas != nullptr)
        ng_molar_mass_ += components_[i].second * xi_ch->M;
      else
        return ERR_INIT_T;
    }
  }
  return ERR_SUCCESS_T;
}

void GasParameters_NG_Gost_dyn::set_p0m() {
  coef_p0m_ = 0.001 * pow(coef_kx_, -3.0) * R * Lt;
}

// calculating
merror_t GasParameters_NG_Gost_dyn::init_kx() {
  coef_kx_ = 0.0;
  const component_characteristics *xi_ch = nullptr;
  for (size_t i = 0; i < components_.size(); ++i) {
    xi_ch = get_characteristics(components_[i].first);
    if (xi_ch == nullptr) {
      error_ = set_error_message(
          ERR_INIT_T, "undefined component in gost model");
      return ERR_INIT_T;
    }
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
  return ERR_SUCCESS_T;
}

merror_t GasParameters_NG_Gost_dyn::init_pseudocrit_vpte() {
  double vol = 0.0;
  double temp = 0.0;
  double press_var = 0.0;
  double tmp_var = 0;
  const component_characteristics *xi_ch = nullptr;
  const component_characteristics *xj_ch = nullptr;
  const critical_params *i_cp = nullptr;
  const critical_params *j_cp = nullptr;
  for (size_t i = 0; i < components_.size(); ++i) {
    xi_ch = get_characteristics(components_[i].first);
    i_cp = get_critical_params(components_[i].first);
    if (i_cp == nullptr)
      return ERR_INIT_T;
    for (size_t j = 0; j < components_.size(); ++j) {
      xj_ch = get_characteristics(components_[j].first);
      j_cp = get_critical_params(components_[j].first);
      if (j_cp == nullptr)
        return ERR_INIT_T;
      tmp_var = components_[i].second * components_[j].second * pow( 
          pow(xi_ch->M / i_cp->density, 0.3333) + pow(xj_ch->M / j_cp->density, 0.3333), 3.0);
      vol += tmp_var;
      temp += tmp_var * sqrt(i_cp->temperature * j_cp->temperature);
    }
    press_var += components_[i].second * i_cp->acentric;
  }
  press_var *= 0.08;
  press_var = 0.291 - press_var;
  pseudocrit_vpte_.volume = 0.125 * vol;
  pseudocrit_vpte_.temperature = 0.125 / pseudocrit_vpte_.volume;
  pseudocrit_vpte_.pressure = 0.001 * R * pseudocrit_vpte_.temperature * 
      press_var / pseudocrit_vpte_.volume;
  return ERR_SUCCESS_T;
}

double GasParameters_NG_Gost_dyn::get_Dn(size_t n) const {
  if (n < 12)
    return Bn_[n] * pow(coef_kx_, -3.0);
  else if (n >= 12 && n < 18)
    return Bn_[n] * pow(coef_kx_, -3.0) - Cn_[n];
  return 0.0;
}

double GasParameters_NG_Gost_dyn::get_Un(size_t n) const {
  if (n < 12)
    return 0.0;
  return Cn_[n];
}

merror_t GasParameters_NG_Gost_dyn::set_volume() {
  if (error_ = check_pt_limits(vpte_.pressure, vpte_.temperature))
    return error_;
  double sigm = sigma_start(),
         tau = vpte_.temperature / Lt;
  double A0 = calculate_A0(sigm);
  double pi_calc = sigm * tau * (1.0 + A0),
         pi = 10e-6 * vpte_.pressure / coef_p0m_;
  int loop_max = 3000;
  while (--loop_max > 0) {
    if (abs(pi_calc - pi) / pi < 10e-6)
      break;
    sigm += calculate_d_sigm(sigm);
    A0 = calculate_A0(sigm);
    pi_calc = sigm * tau * (1.0 + A0);
  }
#ifdef _DEBUG
  if (loop_max == 0)
    assert(0 && "so many roads but assert");
#else
  assert(0);
  // set error, break procedure
#endif  // _DEBUG
  vpte_.volume = pow(coef_kx_, 3.0) / (ng_molar_mass_ * sigm);
  // separate function
  ng_gost_params_.A0 = A0;
  ng_gost_params_.A1 = calculate_A1(sigm);
  ng_gost_params_.A2 = calculate_A2(sigm);
  ng_gost_params_.A3 = calculate_A3(sigm);
  ng_gost_params_.z = 1.0 + A0;
  // function end
  return ERR_SUCCESS_T;
}

merror_t GasParameters_NG_Gost_dyn::check_pt_limits(double p, double t) const {
  /* ckeck pressure[0.1, 30.0]MPa, temperature[250,350]K */
  return ((p >= 100000 && p <= 30000000 ) && (t >= 250 && t <= 350)) ?
      ERR_SUCCESS_T : ERR_INIT_T;
}

merror_t GasParameters_NG_Gost_dyn::set_cp0r() {
  double cp0r = 0.0;
  double tet = Lt / vpte_.temperature;
  auto pow_sinh = [tet] (double C, double D) {
    return C * pow(D * tet / sinh(D * tet), 2.0);};
  auto pow_cosh = [tet] (double C, double D) {
    return C * pow(D * tet / cosh(D * tet), 2.0);};
  const A4_coef *cp_coefs = nullptr;
  for (size_t i = 0; i < components_.size(); ++i) {
    cp_coefs = get_A4_coefs(components_[i].first);
    if (cp_coefs == nullptr)
      return ERR_INIT_T;
    cp0r += components_[i].second * (cp_coefs->B + pow_sinh(cp_coefs->C, cp_coefs->D) +
        pow_cosh(cp_coefs->E, cp_coefs->F) + pow_sinh(cp_coefs->G, cp_coefs->H) + 
        pow_cosh(cp_coefs->I, cp_coefs->J));
  }
  ng_gost_params_.cp0r = cp0r;
  // да, это неправильно и снова отдельная функция должна быть
  ng_gost_params_.k = (1.0 + ng_gost_params_.A1 + pow(1.0 + ng_gost_params_.A2, 2.0) /
      (ng_gost_params_.cp0r - 1.0 + ng_gost_params_.A3)) / ng_gost_params_.z;
  ng_gost_params_.u = 1000 * R * vpte_.temperature * (
      1.0 + ng_gost_params_.A1 + pow(1.0 + ng_gost_params_.A2, 2.0) /
      (ng_gost_params_.cp0r - 1.0 + ng_gost_params_.A3)
      )/ ng_molar_mass_;
  ng_gost_params_.u = sqrt(ng_gost_params_.u);
  return ERR_SUCCESS_T;
}

/* check 07_11_19 */
double GasParameters_NG_Gost_dyn::sigma_start() const {
  return 10e-3 * vpte_.pressure * pow(coef_kx_, 3.0) / (R * vpte_.temperature);
}

/* check 07_11_19 */
double GasParameters_NG_Gost_dyn::calculate_d_sigm(double sigm) const {
  double tau = vpte_.temperature / Lt,
         pi  = 10e-6 * vpte_.pressure / coef_p0m_,
         d_sigm = (pi / tau - (1.0 + calculate_A0(sigm))*sigm) / (1.0 + calculate_A1(sigm));
  return d_sigm;
}

//   dens is sigma, temp is tau
/* check 07_11_19 */
double GasParameters_NG_Gost_dyn::calculate_A0(double sigm) const {
  double A0 = 0.0;
  double tau = vpte_.temperature / Lt;
  for (size_t n = 0; n < A0_3_coefs_count; ++n) {
    const A0_3_coef A3c = A0_3_coefs[n];
    A0 += A3c.a * pow(sigm, A3c.b) * pow(tau, -A3c.u) *
        (A3c.b*get_Dn(n) + (A3c.b - A3c.c*A3c.k*pow(sigm, A3c.k)) * 
            get_Un(n) * exp(-A3c.c * pow(sigm, A3c.k)));
  }
  return A0;
}

/* check 07_11_19 */
double GasParameters_NG_Gost_dyn::calculate_A1(double sigm) const {
  double A1 = 0.0;
  double tau = vpte_.temperature / Lt;
  for (size_t n = 0; n < A0_3_coefs_count; ++n) {
    const A0_3_coef A3c = A0_3_coefs[n];
    A1 += A3c.a * pow(sigm, A3c.b) * pow(tau, -A3c.u) *
      ((A3c.b + 1.0)*A3c.b*get_Dn(n) + ((A3c.b - A3c.c * A3c.k * pow(sigm, A3c.k)) * 
        (A3c.b - A3c.c * A3c.k * pow(sigm, A3c.k) + 1.0) - 
        A3c.c * A3c.k * A3c.k * pow(sigm, A3c.k)) * get_Un(n) * exp(-A3c.c*pow(sigm, A3c.k)));
  }
  return A1;
}

/* check 07_11_19 */
double GasParameters_NG_Gost_dyn::calculate_A2(double sigm) const {
  double A2 = 0.0;
  double tau = vpte_.temperature / Lt;
  for (size_t n = 0; n < A0_3_coefs_count; ++n) {
    const A0_3_coef A3c = A0_3_coefs[n];
    A2 += A3c.a * pow(sigm, A3c.b) * pow(tau, -A3c.u) * (1.0 - A3c.u) *
        (A3c.b*get_Dn(n) + (A3c.b - A3c.c*A3c.k*pow(sigm, A3c.k)) * 
            get_Un(n) * exp(-A3c.c * pow(sigm, A3c.k)));
  }
  return A2;
}

/* check 07_11_19 */
double GasParameters_NG_Gost_dyn::calculate_A3(double sigm) const {
  double A3 = 0.0;
  double tau = vpte_.temperature / Lt;
  for (size_t n = 0; n < A0_3_coefs_count; ++n) {
    const A0_3_coef A3c = A0_3_coefs[n];
    A3 += A3c.a * pow(sigm, A3c.b) * pow(tau, -A3c.u) * (1.0 - A3c.u) * A3c.u *
        (get_Dn(n) + get_Un(n) * exp(-A3c.c * pow(sigm, A3c.k)));
  }
  return A3;
}

void GasParameters_NG_Gost_dyn::csetParameters(
    double v, double p, double t, state_phase) {
  (void)v;
  vpte_.pressure = p;
  vpte_.temperature = t;
  set_volume();
}

double GasParameters_NG_Gost_dyn::cCalculateVolume(double p, double t) {
  parameters bpars = vpte_;
  csetParameters(0.0, p, t, state_phase::GAS);
  double v = vpte_.volume;
  csetParameters(0.0, bpars.pressure, bpars.volume, state_phase::GAS);
  return v;
}
