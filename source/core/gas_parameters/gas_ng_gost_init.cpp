#include "gas_ng_gost_init.h"

#include "gas_ng_gost_defines.h"
#include "models_errors.h"

#include <array>
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
  : GasParameters(prs, cgp, dgp), error_status_(ERR_SUCCESS_T),
    components_(components) {
  if (error_status_ = init_kx())
    return;
  set_V();
  set_Q();
  set_F();
  set_G();
  set_Bn();
  set_Cn();
  if (set_molar_mass()) {
    set_error_message(ERR_INIT_T, "udefined component of natural gas");
    error_status_ = ERR_INIT_T;
    return;
  }
  set_p0m();
  init_pseudocrit_vpte();
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
  // костыли-костылёчки
  const_parameters *cgp = const_parameters::Init(
      GAS_TYPE_MIX, 1.0, 1.0, 1.0, 1.0, 0.1);
  dyn_parameters *dgp = dyn_parameters::Init(
      1.0, 1.0, 1.0, {1.0, 1.0, 1.0});
  if (cgp == NULL || dgp == NULL) {
#ifdef _DEBUG
    assert(0);
#endif  // _DEBUG
    return NULL;
  }
  return new GasParameters_NG_Gost_dyn({0.0, gpi.p, gpi.t},
      *cgp, *dgp, *gpi.const_dyn.ng_gost_components);
}

void GasParameters_NG_Gost_dyn::set_V() {
  double x = 0.0;
  double V = 0.0;
  const component_characteristics *xi_ch = NULL;
  for (int i = 0; i < components_.size(); ++i) {
    xi_ch = get_characteristics(components_[i].first);
    if (xi_ch == NULL) {
      set_error_message(ERR_INIT_T, "undefined component in gost model");
      return;
    }
    V += components_[i].second * pow(xi_ch->E, 2.5);
  }
  V *= V;
  double associate_V_part = 0.0;
  const binary_associate_coef *assoc_coefs = NULL;
  const component_characteristics *xj_ch = NULL;
  for (int i = 0; i < components_.size() - 1; ++i) {
    xi_ch = get_characteristics(components_[i].first);
    for (int j = i + 1; j < components_.size(); ++j) {
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
  const component_characteristics *xi_ch = NULL;
  for (int i = 0; i < components_.size(); ++i) {
    xi_ch = get_characteristics(components_[i].first);
    coef_Q_ += components_[i].second * xi_ch->Q;
  }
}

void GasParameters_NG_Gost_dyn::set_F() {
  coef_F_ = 0.0;
  const component_characteristics *xi_ch = NULL;
  for (int i = 0; i < components_.size(); ++i) {
    xi_ch = get_characteristics(components_[i].first);
    coef_F_ += pow(components_[i].second, 2.0) * xi_ch->F;
  }
}

void GasParameters_NG_Gost_dyn::set_G() {
  double x = 0.0;
  double G = 0.0;
  const component_characteristics *xi_ch = NULL;
  for (int i = 0; i < components_.size(); ++i) {
    xi_ch = get_characteristics(components_[i].first);
    if (xi_ch == NULL) {
      set_error_message(ERR_INIT_T, "undefined component in gost model");
      return;
    }
    G += components_[i].second * xi_ch->G;
  }
  double associate_G_part = 0.0;
  const binary_associate_coef *assoc_coefs = NULL;
  const component_characteristics *xj_ch = NULL;
  for (int i = 0; i < components_.size() - 1; ++i) {
    xi_ch = get_characteristics(components_[i].first);
    for (int j = i + 1; j < components_.size(); ++j) {
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
  const component_characteristics *xi_ch = NULL;
  const component_characteristics *xj_ch = NULL;
  const binary_associate_coef *assoc_coef = NULL;
  for (int n = 0; n < n_max; ++n) {
    // set Bnij
    const A0_3_coef &A3c = A0_3_coefs[n];
    for(int i = 0; i < components_.size(); ++i) {
      xi_ch = get_characteristics(components_[i].first);
      for (int j = 0; j < components_.size(); ++j) {
        xj_ch = get_characteristics(components_[j].first);
        assoc_coef = get_binary_associate_coefs(
            components_[i].first, components_[j].first);
        double Gij = assoc_coef->G * (xi_ch->G + xj_ch->G) / 2.0;
        double Bnij = pow(Gij + 1.0 - A3c.g, A3c.g) * pow(xi_ch->Q*xj_ch->Q + 1.0 - A3c.q, A3c.q) *
            pow(sqrt(xi_ch->F*xj_ch->F) + 1.0 - A3c.f, A3c.f) * pow(xi_ch->S*xj_ch->S + 1.0 - A3c.s, A3c.s) *
            pow(xi_ch->W*xj_ch->W + 1.0 - A3c.w, A3c.w);
        double Eij = assoc_coef->E * sqrt(xi_ch->E*xj_ch->E);
        Bn_[i] += components_[i].second * components_[j].second * Bnij * pow(Eij, A3c.u) *
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
  for (int n = 0; n < n_max; ++n) {
    const A0_3_coef &A3c = A0_3_coefs[n];
    Cn_[n] = pow(coef_G_ +1.0 - A3c.g, A3c.g) * pow(coef_Q_ * coef_Q_  + 1.0 - A3c.q, A3c.q) *
        pow(coef_F_ + 1.0 - A3c.f, A3c.f) * pow(coef_V_, A3c.u);
  }
}

ERROR_TYPE GasParameters_NG_Gost_dyn::set_molar_mass() {
  ng_molar_mass_ = 0.0;
  const component_characteristics *xi_ch = NULL;
  const A9_molar_mass *m_mas = NULL;
  for (int i = 0; i < components_.size(); ++i) {
    xi_ch = get_characteristics(components_[i].first);
    if (xi_ch != NULL) {
      ng_molar_mass_ += components_[i].second * xi_ch->M;
    } else {
      m_mas = get_molar_mass(components_[i].first);
      if (m_mas != NULL)
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
ERROR_TYPE GasParameters_NG_Gost_dyn::init_kx() {
  double x = 0.0;
  coef_kx_ = 0.0;
  const component_characteristics *xi_ch = NULL;
  for (int i = 0; i < components_.size(); ++i) {
    xi_ch = get_characteristics(components_[i].first);
    if (xi_ch == NULL) {
      set_error_message(ERR_INIT_T, "undefined component in gost model");
      return ERR_INIT_T;
    }
    coef_kx_ += components_[i].second * pow(xi_ch->K, 2.5);
  }
  coef_kx_ *= coef_kx_;
  double associate_Kx_part = 0.0;
  const binary_associate_coef *assoc_coefs = NULL;
  const component_characteristics *xj_ch = NULL;
  for (int i = 0; i < components_.size() - 1; ++i) {
    xi_ch = get_characteristics(components_[i].first);
    for (int j = i + 1; j < components_.size(); ++j) {
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

ERROR_TYPE GasParameters_NG_Gost_dyn::init_pseudocrit_vpte() {
  assert(0);
}

double GasParameters_NG_Gost_dyn::get_Dn(int n) const {
  if (n < 13)
    return Bn_[n] * pow(coef_kx_, -3.0);
  else if (n >= 13 && n < 19)
    return Bn_[n] * pow(coef_kx_, -3.0) - Cn_[n];
  return 0.0;
}

double GasParameters_NG_Gost_dyn::get_Un(int n) const {
  if (n < 13)
    return 0.0;
  return Cn_[n];
}

ERROR_TYPE GasParameters_NG_Gost_dyn::set_volume() {
  double sigm = sigma_start(),
         tau = vpte_.temperature / Lt;
  double A0 = calculate_A0(sigm);
  double pi_calc = sigm * tau * (1.0 + A0),
         pi = vpte_.pressure / coef_p0m_;
  int loop_max = 3000;
  while (--loop_max > 0) {
    if ((pi_calc - pi) / pi < 10e-6)
      break;
    sigm += calculate_d_sigm(sigm, A0);
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
  vpte_.volume = 1.0 / (ng_molar_mass_ * pow(coef_kx_, -3.0) * sigm);
  return ERR_SUCCESS_T;
}

double GasParameters_NG_Gost_dyn::sigma_start() const {
  return 1000.0 * vpte_.pressure * pow(coef_kx_, 3.0) / (R * vpte_.pressure);
}

double GasParameters_NG_Gost_dyn::calculate_d_sigm(double sigm, double A0) const {
  double tau = vpte_.temperature / Lt,
         pi  = vpte_.pressure / coef_p0m_,
         d_sigm = (pi / tau - (1.0 + calculate_A0(sigm))*sigm) / (1.0 + calculate_A1(sigm));
  return d_sigm;
}

//   dens is sigma, temp is tau
double GasParameters_NG_Gost_dyn::calculate_A0(double sigm) const {
  double A0 = 0.0;
  double tau = vpte_.temperature / Lt;
  for (int n = 0; n < A0_3_coefs_count; ++n) {
    const A0_3_coef A3c = A0_3_coefs[n];
    A0 += A3c.a * pow(sigm, A3c.b) * pow(tau, -A3c.u) *
        (A3c.b*get_Dn(n) + (A3c.b - A3c.c*A3c.k*pow(sigm, A3c.k)) * 
            get_Un(n) * exp(-A3c.c * pow(sigm, A3c.k)));
  }
  return A0;
}

double GasParameters_NG_Gost_dyn::calculate_A1(double sigm) const {
  double A1 = 0.0;
  double tau = vpte_.temperature / Lt;
  for (int n = 0; n < A0_3_coefs_count; ++n) {
    const A0_3_coef A3c = A0_3_coefs[n];
    A1 += A3c.a * pow(sigm, A3c.b) * pow(tau, -A3c.u) *
      ((A3c.b + 1.0)*A3c.b*get_Dn(n) + ((A3c.b - A3c.c * A3c.k * pow(sigm, A3c.k)) * 
        (A3c.b - A3c.c * A3c.k * pow(sigm, A3c.k) + 1.0) - 
        A3c.c * A3c.k * A3c.k * pow(sigm, A3c.k)) * get_Un(n) * exp(-A3c.c*pow(sigm, A3c.k)));
  }
  return A1;
}

double GasParameters_NG_Gost_dyn::calculate_A2(double sigm) const {
  double A2 = 0.0;
  double tau = vpte_.temperature / Lt;
  for (int n = 0; n < A0_3_coefs_count; ++n) {
    const A0_3_coef A3c = A0_3_coefs[n];
    A2 += A3c.a * pow(sigm, A3c.b) * pow(tau, -A3c.u) * (1.0 - A3c.u) *
        (A3c.b*get_Dn(n) + (A3c.b - A3c.c*A3c.k*pow(sigm, A3c.k)) * 
            get_Un(n) * exp(-A3c.c * pow(sigm, A3c.k)));
  }
  return A2;
}

double GasParameters_NG_Gost_dyn::calculate_A3(double sigm) const {
  double A3 = 0.0;
  double tau = vpte_.temperature / Lt;
  for (int n = 0; n < A0_3_coefs_count; ++n) {
    const A0_3_coef A3c = A0_3_coefs[n];
    A3 += A3c.a * pow(sigm, A3c.b) * pow(tau, -A3c.u) * (1.0 - A3c.u) * A3c.u *
        (get_Dn(n) + get_Un(n) * exp(-A3c.c * pow(sigm, A3c.k)));
  }
  return A3;
}

void GasParameters_NG_Gost_dyn::csetParameters(
    double v, double p, double t, state_phase) {
  assert(0);
}