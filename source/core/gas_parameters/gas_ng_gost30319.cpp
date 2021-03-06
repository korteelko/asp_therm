/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020-2021 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#include "gas_ng_gost30319.h"

#include "asp_utils/Logging.h"
#include "atherm_common.h"
#include "gas_ng_gost_defines.h"

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
  double min, max;
};
/* GOST 30319.3-2015, table2, page 11
 * todo: separate maps for GOST AND ISO */
/**
 * \brief Контейнер допусков молекулярных масс
 *   компонентов газовой смеси для ГОСТ модели
 * */
std::map<gas_t, max_valid_limits_t> mix_valid_molar =
    std::map<gas_t, max_valid_limits_t>{{CH(METHANE), {0.7, 0.99999}},
                                        {CH(ETHANE), {0.0, 0.1}},
                                        {CH(PROPANE), {0.0, 0.035}},
                                        {CH(ALL_BUTANES), {0.0, 0.015}},
                                        {CH(ALL_PENTANES), {0.0, 0.005}},
                                        {CH(HEXANE), {0.0, 0.001}},
                                        {CH(NITROGEN), {0.0, 0.2}},
                                        {CH(CARBON_DIOXIDE), {0.0, 0.2}},
                                        {CH(HELIUM), {0.0, 0.005}},
                                        {CH(HYDROGEN), {0.0, 0.1}},
                                        // SUM of others
                                        {CH(UNDEFINED), {0.0, 0.0015}}};
/**
 * \brief Дополнительный контейнер допусков молекулярных масс
 *   компонентов газовой смеси для ISO модели
 * */
std::map<gas_t, max_valid_limits_t> mix_valid_molar_iso = {
#ifdef ISO_20765
    {CH(ALL_OTHER_ALKANES), {0.0, 0.0005}},
    {CH(HEPTANE), {0.0, 0.0005}},
    {CH(CARBON_MONOXIDE), {0.0, 0.03}},
    {CH(WATER), {0.0, 0.00015}},
    {CH(OXYGEN), {0.0, 0.0002}},
    {CH(HYDROGEN_SULFIDE), {0.0, 0.0002}},
    {CH(ARGON), {0.0, 0.0002}},
#endif  // ISO_20765
};

bool is_valid_limits(
    std::map<const gas_t, max_valid_limits_t>::const_iterator limit_it,
    double part) {
  if ((limit_it->second.min < (part + FLOAT_ACCURACY))
      && ((limit_it->second.max + FLOAT_ACCURACY) > part))
    return true;
  return false;
}

bool is_valid_limits(const std::map<gas_t, max_valid_limits_t>& map_limits,
                     const ng_gost_component& component) {
  const auto limits_it = map_limits.find(component.first);
  if (limits_it == map_limits.cend())
    return false;
  return is_valid_limits(limits_it, component.second);
}

// init check
bool is_valid_limits(const ng_gost_mix& components, bool use_iso20675) {
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
  for (const auto& component : components) {
    const auto limits_it = mix_valid_molar.find(component.first);
    if (limits_it == mix_valid_molar.cend()) {
      if (use_iso20675) {
        // если используем ISO модель, проверим контейнер
        //   с допусками для этой модели
        auto const limits_iso_it = mix_valid_molar_iso.find(component.first);
        if (limits_iso_it != mix_valid_molar_iso.end()) {
          if (is_valid_limits(limits_iso_it, component.second) == false) {
            is_valid = false;
            break;
          }
        }
      }
      if (component.first == CH(ISO_PENTANE)
          || component.first == CH(N_PENTANE)) {
        pentanes.second += component.second;
      } else if (component.first == CH(ISO_BUTANE)
                 || component.first == CH(N_BUTANE)) {
        butanes.second += component.second;
#ifdef ISO_20765
      } else if (component.first == CH(OCTANE) || component.first == CH(NONANE)
                 || component.first == CH(DECANE)) {
        other_alkanes.second += component.second;
#endif  // ISO_20765
      } else {
        others.second += component.second;
      }
      continue;
    }
    if (is_valid_limits(limits_it, component.second) == false) {
      is_valid = false;
      break;
    }
  }
  is_valid &= is_valid_limits(mix_valid_molar, butanes);
  is_valid &= is_valid_limits(mix_valid_molar, pentanes);
#ifdef ISO_20765
  if (use_iso20675) {
    is_valid &= is_valid_limits(mix_valid_molar_iso, other_alkanes);
  } else {
    others.second += other_alkanes.second;
  }
#endif  // ISO_20765
  is_valid &= is_valid_limits(mix_valid_molar, others);
  return is_valid;
}
}  // namespace

GasParametersGost30319Dyn* GasParametersGost30319Dyn::Init(gas_params_input gpi,
                                                           bool use_iso) {
  GasParametersGost30319Dyn* res = nullptr;
#ifndef ISO_20765
  /* Перепроверить валидность опции использования алгоритмов ISO.
   *   Лучше конечно ошибку прокинуть, но можно и так */
  Logging::Append(io_loglvl::warn_logs,
                  "Попытка использовать алгоритмы "
                  "стандарта ISO 20765, вне контекста валидной сборки!\n"
                  "Для их использования пересоберите ПО с опцией ISO_20765");
  use_iso = false;
#endif  // !ISO_20765
  if (!gpi.const_dyn.ng_gost_components->empty()) {
    if (is_valid_limits(*gpi.const_dyn.ng_gost_components, use_iso)) {
      try {
        auto pc = GasParametersGost30319Dyn::calcPseudocriticVPT(
            *gpi.const_dyn.ng_gost_components);
        auto mp = calculate_molar_data(*gpi.const_dyn.ng_gost_components);
        // рассчитать фактор ацентричности в критической точке
        double zk =
            compress_by_volume(pc.pressure, pc.temperature, mp.mass, pc.volume);
        res = new GasParametersGost30319Dyn(
            {0.0, gpi.p, gpi.t}, const_parameters(pc, zk, mp),
            *gpi.const_dyn.ng_gost_components, use_iso);
      } catch (gparameters_exception& e) {
        Logging::Append(io_loglvl::err_logs, e.what());
      }
    } else {
      Logging::Append(ERROR_INIT_T,
                      "natural gas model init error:\n"
                      "components limits check fail\n");
    }
  } else {
    Logging::Append(ERROR_PAIR_DEFAULT(ERROR_INIT_NULLP_ST));
    Logging::Append(io_loglvl::debug_logs,
                    "Пустой список компонентов газовой смеси для ГОСТ модели");
  }
  return res;
}

GasParametersGost30319Dyn::GasParametersGost30319Dyn(parameters prs,
                                                     const_parameters cgp,
                                                     ng_gost_mix components,
                                                     bool use_iso)
    : GasParameters(prs, cgp, dyn_parameters()),
      components_(components),
      use_iso20765_(use_iso) {
  if (setFuncCoefficients())
    set_p0m();
  if (error_.GetErrorCode()) {
    // если была ошибка на стадии инициализации
    status_ = STATUS_HAVE_ERROR;
  } else {
    set_volume();
  }
}

parameters GasParametersGost30319Dyn::calcPseudocriticVPT(
    ng_gost_mix components) {
  double vol = 0.0;
  double temp = 0.0;
  double press_var = 0.0;
  double tmp_var = 0;
  double Mi = 0.0, Mj = 0.0;
  const component_characteristics* x_ch = nullptr;
  const critical_params* i_cp = nullptr;
  const critical_params* j_cp = nullptr;
  for (size_t i = 0; i < components.size(); ++i) {
    if ((x_ch = get_characteristics(components[i].first))) {
      Mi = x_ch->M;
    } else {
      Logging::Append(
          "init pseudocritic by gost model\n"
          "  undefined component: #"
          + std::to_string(components[i].first));
      continue;
    }
    if (!(i_cp = get_critical_params(components[i].first)))
      continue;
    for (size_t j = 0; j < components.size(); ++j) {
      if ((x_ch = get_characteristics(components[j].first))) {
        Mj = x_ch->M;
      } else {
        Logging::Append(
            "init pseudocritic by gost model\n"
            "  undefined component: #"
            + std::to_string(components[j].first));
        continue;
      }
      if (!(j_cp = get_critical_params(components[j].first)))
        continue;
      tmp_var = components[i].second * components[j].second
                * pow(pow(Mi / i_cp->density, 0.333333)
                          + pow(Mj / j_cp->density, 0.333333),
                      3.0);
      vol += tmp_var;
      temp += tmp_var * sqrt(i_cp->temperature * j_cp->temperature);
    }
    press_var += components[i].second * i_cp->acentric;
  }
  press_var *= 0.08;
  press_var = 0.291 - press_var;
  parameters pseudocrit_vpt;
  pseudocrit_vpt.volume = 0.125 * vol;
  pseudocrit_vpt.temperature = 0.125 * temp / vol;
  pseudocrit_vpt.pressure = 1000 * GAS_CONSTANT * pseudocrit_vpt.temperature
                            * press_var / pseudocrit_vpt.volume;
  return pseudocrit_vpt;
}

bool GasParametersGost30319Dyn::setFuncCoefficients() {
  if (!init_kx()) {
    set_V();
    set_Q();
    set_F();
    set_G();
    set_Bn();
    set_Cn();
  }
  return (error_.GetErrorCode()) ? false : true;
}

// calculating
merror_t GasParametersGost30319Dyn::init_kx() {
  coef_kx_ = 0.0;
  const component_characteristics* xi_ch = nullptr;
  for (size_t i = 0; i < components_.size(); ++i) {
    xi_ch = get_characteristics(components_[i].first);
    if (xi_ch == nullptr)
      return error_.SetError(ERROR_INIT_T, "undefined component in gost model");
    coef_kx_ += components_[i].second * pow(xi_ch->K, 2.5);
  }
  coef_kx_ *= coef_kx_;
  double associate_Kx_part = 0.0;
  const binary_associate_coef* assoc_coefs = nullptr;
  const component_characteristics* xj_ch = nullptr;
  for (size_t i = 0; i < components_.size() - 1; ++i) {
    xi_ch = get_characteristics(components_[i].first);
    for (size_t j = i + 1; j < components_.size(); ++j) {
      xj_ch = get_characteristics(components_[j].first);
      assoc_coefs = get_binary_associate_coefs(components_[i].first,
                                               components_[j].first);
      associate_Kx_part +=
          components_[i].second * components_[j].second
          * ((pow(assoc_coefs->K, 5.0) - 1.0) * pow(xi_ch->K * xj_ch->K, 2.5));
    }
  }
  associate_Kx_part *= 2.0;
  coef_kx_ += associate_Kx_part;
  coef_kx_ = pow(coef_kx_, 0.2);
  return ERROR_SUCCESS_T;
}

void GasParametersGost30319Dyn::set_V() {
  double V = 0.0;
  const component_characteristics* xi_ch = nullptr;
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
  const binary_associate_coef* assoc_coefs = nullptr;
  const component_characteristics* xj_ch = nullptr;
  for (size_t i = 0; i < components_.size() - 1; ++i) {
    xi_ch = get_characteristics(components_[i].first);
    for (size_t j = i + 1; j < components_.size(); ++j) {
      xj_ch = get_characteristics(components_[j].first);
      assoc_coefs = get_binary_associate_coefs(components_[i].first,
                                               components_[j].first);
      associate_V_part +=
          components_[i].second * components_[j].second
          * ((pow(assoc_coefs->V, 5.0) - 1.0) * pow(xi_ch->E * xj_ch->E, 2.5));
    }
  }
  associate_V_part *= 2.0;
  V += associate_V_part;
  V = pow(V, 0.2);
  coef_V_ = V;
}

void GasParametersGost30319Dyn::set_Q() {
  coef_Q_ = 0.0;
  const component_characteristics* xi_ch = nullptr;
  for (size_t i = 0; i < components_.size(); ++i) {
    xi_ch = get_characteristics(components_[i].first);
    coef_Q_ += components_[i].second * xi_ch->Q;
  }
}

void GasParametersGost30319Dyn::set_F() {
  coef_F_ = 0.0;
  const component_characteristics* xi_ch = nullptr;
  for (size_t i = 0; i < components_.size(); ++i) {
    xi_ch = get_characteristics(components_[i].first);
    coef_F_ += pow(components_[i].second, 2.0) * xi_ch->F;
  }
}

void GasParametersGost30319Dyn::set_G() {
  double G = 0.0;
  const component_characteristics* xi_ch = nullptr;
  for (size_t i = 0; i < components_.size(); ++i) {
    xi_ch = get_characteristics(components_[i].first);
    if (xi_ch == nullptr) {
      error_.SetError(ERROR_INIT_T, "undefined component in gost model");
      return;
    }
    G += components_[i].second * xi_ch->G;
  }
  double associate_G_part = 0.0;
  const binary_associate_coef* assoc_coefs = nullptr;
  const component_characteristics* xj_ch = nullptr;
  for (size_t i = 0; i < components_.size() - 1; ++i) {
    xi_ch = get_characteristics(components_[i].first);
    for (size_t j = i + 1; j < components_.size(); ++j) {
      xj_ch = get_characteristics(components_[j].first);
      assoc_coefs = get_binary_associate_coefs(components_[i].first,
                                               components_[j].first);
      associate_G_part += components_[i].second * components_[j].second
                          * ((assoc_coefs->G - 1.0) * (xi_ch->G + xj_ch->G));
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
  const component_characteristics* xi_ch = nullptr;
  const component_characteristics* xj_ch = nullptr;
  const binary_associate_coef* assoc_coef = nullptr;
  for (size_t n = 0; n < n_max; ++n) {
    // set Bnij
    const A0_3_coef& A3c = A0_3_coefs[n];
    for (size_t i = 0; i < components_.size(); ++i) {
      xi_ch = get_characteristics(components_[i].first);
      for (size_t j = 0; j < components_.size(); ++j) {
        xj_ch = get_characteristics(components_[j].first);
        assoc_coef = get_binary_associate_coefs(components_[i].first,
                                                components_[j].first);
        double Gij = assoc_coef->G * (xi_ch->G + xj_ch->G) / 2.0;
        double Bnij = pow(Gij + 1.0 - A3c.g, A3c.g)
                      * pow(xi_ch->Q * xj_ch->Q + 1.0 - A3c.q, A3c.q)
                      * pow(sqrt(xi_ch->F * xj_ch->F) + 1.0 - A3c.f, A3c.f)
                      * pow(xi_ch->S * xj_ch->S + 1.0 - A3c.s, A3c.s)
                      * pow(xi_ch->W * xj_ch->W + 1.0 - A3c.w, A3c.w);
        double Eij = assoc_coef->E * sqrt(xi_ch->E * xj_ch->E);
        Bn_[n] += components_[i].second * components_[j].second * Bnij
                  * pow(Eij, A3c.u) * pow(xi_ch->K * xj_ch->K, 1.5);
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
    const A0_3_coef& A3c = A0_3_coefs[n];
    Cn_[n] = pow(coef_G_ + 1.0 - A3c.g, A3c.g)
             * pow(coef_Q_ * coef_Q_ + 1.0 - A3c.q, A3c.q)
             * pow(coef_F_ + 1.0 - A3c.f, A3c.f) * pow(coef_V_, A3c.u);
  }
}

void GasParametersGost30319Dyn::set_p0m() {
  coef_p0m_ = 0.001 * pow(coef_kx_, -3.0) * GAS_CONSTANT * Lt;
}

merror_t GasParametersGost30319Dyn::set_cp0r() {
  merror_t error = ERROR_SUCCESS_T;
  double cp0r = 0.0;
  const double tet = Lt / vpte_.temperature;
  auto pow_sinh = [tet](double C, double D) {
    // for carbon monoxide we get nan
    return (is_equal(D, 0.0)) ? 0.0 : C * pow(D * tet / sinh(D * tet), 2.0);
  };
  auto pow_cosh = [tet](double C, double D) {
    return C * pow(D * tet / cosh(D * tet), 2.0);
  };
  const A4_coef* cp_c = nullptr;
  const component_characteristics* x_ch = nullptr;
  for (size_t i = 0; i < components_.size(); ++i) {
    cp_c = get_A4_coefs(components_[i].first);
    x_ch = get_characteristics(components_[i].first);
    if (cp_c != nullptr && x_ch != nullptr) {
      // by Appex B of ISO 20765
      if (gas_char::IsNoble(components_[i].first)) {
        /* for noble gasses available only B,
         *   see M. Jaeschke, P. Schley
         *   "Ideal-Gas Thermodynamic Properties for
         *        Natural-Gas Applications"
         * also, for oxygen, carbon monoxide available only B, C and D
         *   coeficients
         */
        cp0r += components_[i].second * cp_c->B;
      } else {
        cp0r +=
            components_[i].second
            * (cp_c->B + pow_sinh(cp_c->C, cp_c->D) + pow_cosh(cp_c->E, cp_c->F)
               + pow_sinh(cp_c->G, cp_c->H) + pow_cosh(cp_c->I, cp_c->J))
            * x_ch->M / (1000.0 * GAS_CONSTANT);
      }
    } else {
      error = error_.SetError(ERROR_INIT_T,
                              "ГОСТ модель: cp0r, не распознан компонент");
    }
  }
  ng_gost_params_.cp0r = cp0r * const_params.mp.Rm;
  return error;
}

#if defined(ISO_20765)
merror_t GasParametersGost30319Dyn::set_fi0r() {
  merror_t error = ERROR_SUCCESS_T;
  double fi0r = 0.0, fi0r_t = 0.0;
  double fi0r_tt = 0.0;
  const double tau = Lt / vpte_.temperature, tauT = Lt / 298.15;
  const double sigm = calculate_sigma(vpte_.pressure, vpte_.temperature),
               sigmT = calculate_sigma(101325.0, 298.15);
  auto pow_sinh = [tau](double C, double D) {
    return (is_equal(D, 0.0)) ? 0.0 : C * pow(D / sinh(D * tau), 2.0);
  };
  auto pow_cosh = [tau](double C, double D) {
    return C * pow(D / cosh(D * tau), 2.0);
  };
  if (is_above0(sigm) && is_above0(sigmT)) {
    const double appendix = std::log(tauT / tau) + std::log(sigm / sigmT);
    auto ln_sinh = [tau](double C, double D) {
      return (is_equal(D, 0.0)) ? 0.0 : C * std::log(sinh(D * tau));
    };
    auto ln_cosh = [tau](double C, double D) {
      return C * std::log(cosh(D * tau));
    };
    auto sinh_cosh = [tau](double C, double D) {
      return C * D * sinh(D * tau) / cosh(D * tau);
    };
    auto cosh_sinh = [tau](double C, double D) {
      return (is_equal(D, 0.0)) ? 0.0 : C * D * cosh(D * tau) / sinh(D * tau);
    };
    const A4_coef* cpc = nullptr;
    for (size_t i = 0; i < components_.size(); ++i) {
      cpc = get_A4_coefs(components_[i].first);
      if (cpc != nullptr) {
        // by Appex B of ISO 20765
        fi0r += components_[i].second
                * (cpc->A1 + cpc->A2 * tau + cpc->B * std::log(tau)
                   + ln_sinh(cpc->C, cpc->D) - ln_cosh(cpc->E, cpc->F)
                   + ln_sinh(cpc->G, cpc->H) - ln_cosh(cpc->I, cpc->J)
                   + std::log(components_[i].second));
        fi0r_t += components_[i].second
                  * (cpc->A2 + (cpc->B - 1.0) / tau + cosh_sinh(cpc->C, cpc->D)
                     - sinh_cosh(cpc->E, cpc->F) + cosh_sinh(cpc->G, cpc->H)
                     - sinh_cosh(cpc->I, cpc->J));
        fi0r_tt += components_[i].second
                   * (-(cpc->B - 1.0) / tau / tau - pow_sinh(cpc->C, cpc->D)
                      - pow_cosh(cpc->E, cpc->F) - pow_sinh(cpc->G, cpc->H)
                      - pow_cosh(cpc->I, cpc->J));
      } else {
        error = error_.SetError(ERROR_INIT_T,
                                "ГОСТ модель: fi0r, не распознан компонент");
        break;
      }
    }
    ng_gost_params_.fi0r = fi0r + appendix;
    ng_gost_params_.fi0r_t = fi0r_t;
    ng_gost_params_.fi0r_tt = fi0r_tt;
  } else {
    error = error_.SetError(ERROR_INIT_T,
                            "ГОСТ модель: fi0r, результат "
                            "расчёта приведённой плотности некорректен.");
  }
  return error;
}
#endif  // ISO_20765

merror_t GasParametersGost30319Dyn::set_volume() {
  merror_t error = ERROR_INIT_T;
  if (status_ != STATUS_HAVE_ERROR) {
    if (inLimits(vpte_.pressure, vpte_.temperature)) {
      double sigma = calculate_sigma(vpte_.pressure, vpte_.temperature);
      vpte_.volume = pow(coef_kx_, 3.0) / (const_params.mp.mass * sigma);
      bool is_valid = (set_cp0r() == ERROR_SUCCESS_T);
#if defined(ISO_20765)
      if (use_iso20765_ && is_valid) {
        if ((is_valid = (set_fi0r() == ERROR_SUCCESS_T))) {
          set_iso_params(sigma);
        }
      } else
#endif  // ISO_20765
      {
        if (is_valid)
          set_gost_params(sigma);
      }
      update_dynamic();
    } else {
      Logging::Append(ERROR_CALCULATE_T, "check ng_gost limits");
      status_ = STATUS_NOT;
      error = ERROR_CALCULATE_T;
    }
  }
  // function end
  return error;
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
  const A6_coef* coef = nullptr;
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
  for (size_t n = 0; n < 18; ++n) {
    ng_gost_params_.B +=
        A0_3_coefs[n].a * Bn_[n] * std::pow(tau, A0_3_coefs[n].u);
  }
}

void GasParametersGost30319Dyn::set_fi(double t, double sigma) {
  double tau = Lt / t;
  ng_gost_params_.fi = ng_gost_params_.fi0r
                       + ng_gost_params_.B * sigma / std::pow(coef_kx_, 3.0);
  double c1 = 0.0, c2 = 0.0;
  for (size_t n = 12; n < 18; ++n)
    c1 += A0_3_coefs[n].a * Cn_[n] * std::pow(tau, A0_3_coefs[n].u);
  c1 *= sigma;
  for (size_t n = 12; n < 58; ++n)
    c2 += A0_3_coefs[n].a * Cn_[n] * std::pow(tau, A0_3_coefs[n].u)
          * std::pow(sigma, A0_3_coefs[n].b)
          * std::exp(-A0_3_coefs[n].c * std::pow(sigma, A0_3_coefs[n].k));
  ng_gost_params_.fi += -c1 + c2;
}

/* наверное излишне оптимизировано */
void GasParametersGost30319Dyn::set_fi_der(double t, double sigma) {
  double tau = Lt / t;
  double s_k = sigma / pow(coef_kx_, 3.0);
  double fi_t = tau * ng_gost_params_.fi0r_t,
         fi_tt = tau * tau * ng_gost_params_.fi0r_tt,
         fi_d = 1.0 + ng_gost_params_.B * s_k,
         fi_1 = 1.0 + 2.0 * ng_gost_params_.B * s_k, fi_2 = 1.0;
  double dfi_t = 0.0, dfi_tt = 0.0, dfi_d = 0.0, dfi_1 = 0.0, dfi_2 = 0.0;

  // 1 - 18
  for (size_t n = 0; n < 18; ++n) {
    const A0_3_coef& A3c = A0_3_coefs[n];
    double d1 = A3c.a * Bn_[n] * pow(tau, A3c.u);
    double d2 = (A3c.u - 1.0) * d1;
    dfi_t += A3c.u * d1;
    dfi_tt += A3c.u * d2;
    dfi_2 += -d2;
  }
  fi_t += s_k * dfi_t;
  fi_tt += s_k * dfi_tt;
  fi_2 += s_k * dfi_2;

  // 13 - 18
  dfi_t = 0.0, dfi_tt = 0.0, dfi_d = 0.0, dfi_1 = 0.0, dfi_2 = 0.0;
  for (size_t n = 12; n < 18; ++n) {
    const A0_3_coef& A3c = A0_3_coefs[n];
    double d1 = A3c.a * Cn_[n] * pow(tau, A3c.u);
    double d2 = (A3c.u - 1.0) * d1;
    dfi_t += A3c.u * d1;
    dfi_tt += A3c.u * d2;
    dfi_d += d1;
    dfi_1 += d1;
    dfi_2 += -d2;
  }
  fi_t += -sigma * dfi_t;
  fi_tt += -sigma * dfi_tt;
  fi_d += -sigma * dfi_d;
  fi_1 += -2.0 * sigma * dfi_1;
  fi_2 += -sigma * dfi_2;

  // 13 - 58
  dfi_t = 0.0, dfi_tt = 0.0, dfi_d = 0.0, dfi_1 = 0.0, dfi_2 = 0.0;
  for (size_t n = 12; n < 58; ++n) {
    const A0_3_coef& A3c = A0_3_coefs[n];
    double d1 = A3c.a * Cn_[n] * pow(tau, A3c.u) * pow(sigma, A3c.b)
                * exp(-A3c.c * pow(sigma, A3c.k));
    double d2 = A3c.u * (A3c.u - 1.0) * d1;
    double k3 = A3c.b - A3c.c * A3c.k * pow(sigma, A3c.k);
    double d3 = d1 * k3;
    double d4 =
        d1 * (k3 - pow(A3c.k, 2.0) * A3c.c * pow(sigma, A3c.k) + pow(k3, 2.0));
    double d5 = d3 * (1.0 - A3c.u);
    dfi_t += A3c.u * d1;
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
  ng_gost_params_.fi_d = fi_d / sigma;
  ng_gost_params_.fi_1 = fi_1;
  ng_gost_params_.fi_2 = fi_2;
}
#endif  // ISO_20765

double GasParametersGost30319Dyn::calculate_sigma(double p, double t) const {
  double sigm = sigma_start(p, t), tau = t / Lt;
  double A0 = calculate_A0(t, sigm);
  // todo: remove magic numbers!!!
  double pi_calc = sigm * tau * (1.0 + A0), pi = 0.000001 * p / coef_p0m_;
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
  std::map<dyn_setup, double> params;
  double Rm = const_params.mp.Rm;
#if defined(ISO_20765)
  ng_gost_params_.u = 0.0;
  ng_gost_params_.h = 0.0;
  ng_gost_params_.s = 0.0;
  ng_gost_params_.cv = 0.0;
  ng_gost_params_.cp = 0.0;
  if (use_iso20765_) {
    double tau = Lt / vpte_.temperature;
    ng_gost_params_.u = ng_gost_params_.fi_t * Rm;
    ng_gost_params_.h =
        ng_gost_params_.u + (ng_gost_params_.z) * Rm * vpte_.temperature;
    ng_gost_params_.s = (tau * ng_gost_params_.fi_t - ng_gost_params_.fi) * Rm;
    ng_gost_params_.cv = -tau * tau * ng_gost_params_.fi_tt * Rm;
    ng_gost_params_.cp = ng_gost_params_.cv
                         + ng_gost_params_.fi_2 * ng_gost_params_.fi_2 * Rm
                               / ng_gost_params_.fi_1;
    ng_gost_params_.k = ng_gost_params_.fi_1 * ng_gost_params_.cp
                        / (ng_gost_params_.z * ng_gost_params_.cv);
    ng_gost_params_.w = std::sqrt(ng_gost_params_.z * ng_gost_params_.k
                                  * vpte_.temperature * Rm);
  } else
#endif  // ISO_20765
  {
    auto t = 1.0 + ng_gost_params_.A1
             + std::pow(1.0 + ng_gost_params_.A2, 2.0)
                   / (ng_gost_params_.cp0r - 1.0 + ng_gost_params_.A3);
    ng_gost_params_.k = t / ng_gost_params_.z;
    ng_gost_params_.w = sqrt(Rm * t * vpte_.temperature);
  }
  params = {
#if defined(ISO_20765)
    {DYNAMIC_HEAT_CAP_PRES, ng_gost_params_.cp},
    {DYNAMIC_HEAT_CAP_VOL, ng_gost_params_.cv},
    {DYNAMIC_INTERNAL_ENERGY, ng_gost_params_.u},
    {DYNAMIC_ENTALPHY, ng_gost_params_.h},
    {DYNAMIC_ENTROPY, ng_gost_params_.s},
#endif  // ISO_20765
    {DYNAMIC_ADIABATIC, ng_gost_params_.k}
  };
  dyn_params_.ResetParameters(vpte_, params);
}

merror_t GasParametersGost30319Dyn::inLimits(double p, double t) {
  /* ckeck pressure[0.1, 30.0]MPa, temperature[250,350]K */
  return gost_30319_within(p, t);
}

/* check 07_11_19 */
double GasParametersGost30319Dyn::sigma_start(double p, double t) const {
  return 0.001 * p * pow(coef_kx_, 3.0) / (GAS_CONSTANT * t);
}

/* check 07_11_19 */
double GasParametersGost30319Dyn::calculate_d_sigm(double t,
                                                   double p,
                                                   double sigm) const {
  double tau = t / Lt, pi = 0.000001 * p / coef_p0m_,
         d_sigm = (pi / tau - (1.0 + calculate_A0(t, sigm)) * sigm)
                  / (1.0 + calculate_A1(t, sigm));
  return d_sigm;
}

//   dens is sigma, temp is tau
/* check 07_11_19 */
double GasParametersGost30319Dyn::calculate_A0(double t, double sigm) const {
  double A0 = 0.0;
  double tau = t / Lt;
  for (size_t n = 0; n < A0_3_coefs_count; ++n) {
    const A0_3_coef& A3c = A0_3_coefs[n];
    A0 += A3c.a * pow(sigm, A3c.b) * pow(tau, -A3c.u)
          * (A3c.b * get_Dn(n)
             + (A3c.b - A3c.c * A3c.k * pow(sigm, A3c.k)) * get_Un(n)
                   * exp(-A3c.c * pow(sigm, A3c.k)));
  }
  return A0;
}

/* check 07_11_19 */
double GasParametersGost30319Dyn::calculate_A1(double t, double sigm) const {
  double A1 = 0.0;
  double tau = t / Lt;
  for (size_t n = 0; n < A0_3_coefs_count; ++n) {
    const A0_3_coef& A3c = A0_3_coefs[n];
    A1 += A3c.a * pow(sigm, A3c.b) * pow(tau, -A3c.u)
          * ((A3c.b + 1.0) * A3c.b * get_Dn(n)
             + ((A3c.b - A3c.c * A3c.k * pow(sigm, A3c.k))
                    * (A3c.b - A3c.c * A3c.k * pow(sigm, A3c.k) + 1.0)
                - A3c.c * A3c.k * A3c.k * pow(sigm, A3c.k))
                   * get_Un(n) * exp(-A3c.c * pow(sigm, A3c.k)));
  }
  return A1;
}

/* check 07_11_19 */
double GasParametersGost30319Dyn::calculate_A2(double t, double sigm) const {
  double A2 = 0.0;
  double tau = t / Lt;
  for (size_t n = 0; n < A0_3_coefs_count; ++n) {
    const A0_3_coef& A3c = A0_3_coefs[n];
    A2 += A3c.a * pow(sigm, A3c.b) * pow(tau, -A3c.u) * (1.0 - A3c.u)
          * (A3c.b * get_Dn(n)
             + (A3c.b - A3c.c * A3c.k * pow(sigm, A3c.k)) * get_Un(n)
                   * exp(-A3c.c * pow(sigm, A3c.k)));
  }
  return A2;
}

/* check 07_11_19 */
double GasParametersGost30319Dyn::calculate_A3(double t, double sigm) const {
  double A3 = 0.0;
  double tau = t / Lt;
  for (size_t n = 0; n < A0_3_coefs_count; ++n) {
    const A0_3_coef& A3c = A0_3_coefs[n];
    A3 += A3c.a * pow(sigm, A3c.b) * pow(tau, -A3c.u) * (1.0 - A3c.u) * A3c.u
          * (get_Dn(n) + get_Un(n) * exp(-A3c.c * pow(sigm, A3c.k)));
  }
  return A3;
}

void GasParametersGost30319Dyn::csetParameters(double v,
                                               double p,
                                               double t,
                                               state_phase) {
  (void)v;
  vpte_.pressure = p;
  vpte_.temperature = t;
  set_volume();
}

double GasParametersGost30319Dyn::cCalculateVolume(double p, double t) {
  parameters bpars = vpte_;
  csetParameters(0.0, p, t, state_phase::GAS);
  double v = vpte_.volume;
  csetParameters(0.0, bpars.pressure, bpars.temperature, state_phase::GAS);
  return v;
}

bool GasParametersGost30319Dyn::IsValid() {
  return gost_30319_within(vpte_.pressure, vpte_.temperature);
}

bool GasParametersGost30319Dyn::IsValid(parameters prs) {
  return gost_30319_within(prs.pressure, prs.temperature);
}
