/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020-2021 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#include "gasmix_init.h"

#include "ErrorWrap.h"
#include "Logging.h"
#include "atherm_common.h"
#include "model_general.h"

#include <algorithm>
#include <cmath>
#include <numeric>
#include <vector>

#include <assert.h>
#include <string.h>

gasmix_component_info::gasmix_component_info(const std::string& name,
                                             const std::string& path,
                                             const double part)
    : name(name), path(path), part(part) {}

bool operator<(const gasmix_component_info& lg,
               const gasmix_component_info& rg) {
  return strcmp(lg.name.c_str(), rg.name.c_str()) <= 0;
}

namespace ns_avg {
/* todo: осталось несколько неясных моментов - по поводу vk и zk
 *   когда разберусь - верну  */
#ifdef UNDEFINED_DEFINE
/** \brief Рассчитать среднюю критическую температуру
 *   простым методом(глава 4.2) */
double dfl_avg_Tk(const parameters_mix& components) {
  double tk = 0.0;
  for (auto const& x : components)
    tk += x.first * x.second.first.T_K;
  return tk;
}
/** \brief Рассчитать среднее критическое давление
 *   по правилу Праусница-Ганна(глава 4.2) */
double dfl_avg_Pk(const parameters_mix& components, double R, double tk) {
  double zk = 0.0, vk = 0.0;
  for (auto const& x : components) {
    zk += x.first * x.second.first.Z_K;
    vk += x.first * x.second.first.V_K;
  }
  return R * zk * tk / vk;
}
/** \brief assert doesn't use it! Have not proof.
 *    Рассчитать среднее значение фактора ацентричности */
// todo: неверный метод(как мне кажется)
//   по книге х используется, а не y
double dfl_avg_acentric(const parameters_mix& components) {
  double w = 0.0;
  for (auto const& x : components)
    w += x.first * x.second.first.acentricfactor;
  return w;
}
#endif  // 0
/* todo: про критические параметры для разных уравнений состояний
 *   можно почитать в этой же книге, или у Бруссиловского.
 *   По правилу Лоренца-Бертло можно попридумывать функции и для
 *   других моделей. */
double rk2_avg_Tk(const parameters_mix& components) {
  double num = 0.0;
  double dec = 0.0;
  for (auto const& x : components) {
    auto critical = x.second.first.critical;
    num += x.first * sqrt(pow(critical.temperature, 2.5) / critical.pressure);
    dec += x.first * critical.temperature / critical.pressure;
  }
  return pow(num, 1.3333) / pow(dec, 0.6667);
}
double rk2_avg_Pk(const parameters_mix& components) {
  double num = 0.0;
  double dec = 0.0;
  for (auto const& x : components) {
    auto critical = x.second.first.critical;
    num += x.first * sqrt(pow(critical.temperature, 2.5) / critical.pressure);
    dec += x.first * critical.temperature / critical.pressure;
  }
  return pow(num, 1.3333) / pow(dec, 1.6667);
}
double rk2_avg_Zk() {
  return 0.3333;
}
double rk2_avg_acentric(const parameters_mix& components) {
  double w = 0.0;
  for (auto const& x : components)
    w += x.first * x.second.first.acentricfactor;
  return w;
}

/* истинные параметры критической точки смесей
 *   "Свойства газов и жидкостей" Рида, Праусница, Шервуда глава 5.7 */
double lee_avg_Tk(const parameters_mix& components) {
  double psy_dec = std::accumulate(
      components.begin(), components.end(), 0.0,
      [](double a, const std::pair<const double, const_dyn_parameters>& c) {
        return a
               + c.first * c.second.first.critical.volume
                     * c.second.first.mp.mass;
      });
  double tk = std::accumulate(
      components.begin(), components.end(), 0.0,
      [psy_dec](double a,
                const std::pair<const double, const_dyn_parameters>& c) {
        return a
               + c.first * c.second.first.critical.volume
                     * c.second.first.critical.temperature
                     * c.second.first.mp.mass / psy_dec;
      });
  return tk;
}

/** \brief Получить коэффициенты функции расчитывания крит. температуры
 *   для d <= 0.5 (lh в названии - less half) */
static std::array<double, 5> ch_pr_psy_tk_coefs_lh(gas_t i, gas_t j) {
  if (gas_char::IsAromatic(i) || gas_char::IsAromatic(j)) {
    return {-0.0219, 1.227, -24.277, 147.673, -259.433};
  } else {
    if (gas_char::IsHydrogenSulfide(i) || gas_char::IsHydrogenSulfide(j)) {
      return {-0.0479, -5.725, 70.974, -161.319, 0.0};
    } else {
      if (gas_char::IsCarbonDioxide(i) || gas_char::IsCarbonDioxide(j)) {
        return {-0.0953, 2.185, -33.985, 179.068, -264.522};
      } else {
        if (gas_char::IsAcetylene(i) || gas_char::IsAcetylene(j)) {
          return {-0.0077, -0.095, -0.225, 3.528, 0.0};
        } else {
          if (gas_char::IsCarbonMonoxide(i) || gas_char::IsCarbonMonoxide(j))
            return {-0.0076, 0.286, -1.343, 5.443, -3.038};
        }
      }
    }
  }
  return {-0.0076, 0.287, -1.343, 5.443, -3.038};
}
/* todo:
 *   1) don't use: таблица не полная, а толька из РПШ для
 *     0.0 <= d <= 0.5]
 *   2) лишний пересчёт убрать - здесь зеркальны i и j */
double ch_pr_avg_Tk(const parameters_mix& components) {
  double teta_dec = std::accumulate(
      components.begin(), components.end(), 0.0,
      [](double a, const std::pair<const double, const_dyn_parameters>& c) {
        return a
               + c.first
                     * std::pow(c.second.first.mp.mass
                                    * c.second.first.critical.volume,
                                0.666667);
      });
  double teta_i, teta_j;
  double d;
  double tau;
  double dtau;
  double tk = 0.0;
  int i = 0, j = 0;
  for (auto const& x : components) {
    const auto& const_px = x.second.first;
    teta_i = x.first
             * std::pow(const_px.mp.mass * const_px.critical.volume, 0.666667)
             / teta_dec;
    j = 0;
    dtau = 0.0;
    for (auto const& y : components) {
      const auto& const_py = y.second.first;
      if (j > i) {
        teta_j = y.first
                 * std::pow(y.second.first.mp.mass * const_py.critical.volume,
                            0.666667)
                 / teta_dec;
        d = std::abs(const_px.critical.temperature
                     - const_py.critical.temperature)
            / (const_px.critical.temperature + const_py.critical.temperature);
        std::array<double, 5> psy_c;
        if (d <= 0.5 + FLOAT_ACCURACY)
          psy_c = ch_pr_psy_tk_coefs_lh(const_px.gas_name, const_py.gas_name);
        tau = (const_px.critical.temperature + const_py.critical.temperature)
              * (psy_c[0] + psy_c[1] * d + psy_c[2] * d * d
                 + psy_c[3] * std::pow(d, 3.0) + psy_c[4] * std::pow(d, 4.0))
              * 0.5;
        dtau += teta_i * teta_j * tau;
      }
      j++;
    }
    tk += teta_i * const_px.critical.temperature + 2.0 * dtau;
    i++;
  }
  return tk;
}
/** \brief Получить коэффициенты функции расчитывания крит. объёма
 *   для d <= 0.5 (lh в названии - less half) */
static std::array<double, 5> ch_pr_psy_vk_coefs_lh(gas_t i, gas_t j) {
  // если оба ароматические:
  if (gas_char::IsAromatic(i) && gas_char::IsAromatic(j)) {
    return {0.0, 0.0, 0.0, 0.0, 0.0};
  } else {
    if (gas_char::IsCycleParafine(i) || gas_char::IsCycleParafine(j)) {
      return {0.0, 0.0, 0.0, 0.0, 0.0};
    } else {
      // там про парафин нормального строения, а я про обычный углеводород
      if ((gas_char::IsHydrocarbon(i) && gas_char::IsAromatic(j))
          || (gas_char::IsHydrocarbon(j) && gas_char::IsAromatic(i))) {
        return {0.0753, -3.332, 2.220, 0.0, 0.0};
      } else {
        if (gas_char::IsHydrogenSulfide(i) || gas_char::IsHydrogenSulfide(j)
            || gas_char::IsCarbonDioxide(i) || gas_char::IsCarbonDioxide(j)) {
          return {-0.4957, 17.1185, -168.56, 587.05, -698.89};
        }
      }
    }
  }
  return {0.1397, -2.9672, 1.8337, -1.536, 0.0};
}

double ch_pr_avg_Vk(const parameters_mix& components) {
  double teta_dec = std::accumulate(
      components.begin(), components.end(), 0.0,
      [](double a, const std::pair<const double, const_dyn_parameters>& c) {
        return a
               + c.first
                     * std::pow(c.second.first.mp.mass
                                    * c.second.first.critical.volume,
                                0.666667);
      });
  double teta_i, teta_j;
  double d;
  double nu;
  double dnu;
  double vk = 0.0;
  int i = 0, j = 0;
  for (auto const& x : components) {
    // m3/kg -> m3/mol
    double xvk = x.second.first.mp.mass * x.second.first.critical.volume;
    teta_i = x.first * std::pow(xvk, 0.666667) / teta_dec;
    j = 0;
    dnu = 0.0;
    for (auto const& y : components) {
      if (j > i) {
        double yvk = y.second.first.mp.mass * y.second.first.critical.volume;
        teta_j = y.first * std::pow(yvk, 0.666667) / teta_dec;
        d = std::abs(std::pow(xvk, 0.666667) - std::pow(yvk, 0.666667))
            / (std::pow(xvk, 0.666667) + std::pow(yvk, 0.666667));
        std::array<double, 5> psy_c;
        if (d <= 0.5 + FLOAT_ACCURACY)
          psy_c = ch_pr_psy_vk_coefs_lh(x.second.first.gas_name,
                                        y.second.first.gas_name);
        nu = (xvk + yvk)
             * (psy_c[0] + psy_c[1] * d + psy_c[2] * d * d
                + psy_c[3] * std::pow(d, 3.0) + psy_c[4] * std::pow(d, 4.0))
             * 0.5;
        dnu += teta_i * teta_j * nu;
      }
      j++;
    }
    vk += teta_i * xvk + 2.0 * dnu;
    i++;
  }
  double av_mol = std::accumulate(
      components.begin(), components.end(), 0.0,
      [](double a, const std::pair<const double, const_dyn_parameters>& c) {
        return a + c.first * c.second.first.mp.mass;
      });
  return vk / av_mol;
}

constexpr int index_pk = 0;
constexpr int index_tk = 1;
constexpr int index_vk = 2;
constexpr int index_zk = 3;
constexpr int index_mol = 4;
constexpr int index_accent = 5;
/** \brief Получить массив средних значений(глава 4.2)
 *   [P_k, T_k, V_k, Z_k, mol, acentric]*/
std::array<double, 6> get_average_params(const parameters_mix& components,
                                         const model_str& ms) {
  std::array<double, 6> avg_val = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  for (auto const& x : components) {
    // молярная масса
    avg_val[index_mol] += x.first * x.second.first.mp.mass;
  }
  // тут разграничение по моделям, если руки дойдут
  //   классическая двухпараметрическая модель Редлиха-Квонга
  if (ms.model_type.type == rg_model_t::REDLICH_KWONG
      && ms.model_type.subtype == MODEL_SUBTYPE_DEFAULT) {
    avg_val[index_pk] = rk2_avg_Pk(components);
    avg_val[index_tk] = rk2_avg_Tk(components);
    avg_val[index_zk] = rk2_avg_Zk();
    avg_val[index_accent] = rk2_avg_acentric(components);
    avg_val[index_vk] = 0.0;
  } else {
#ifdef UNDEFINED_DEFINE
    avg_val[index_tk] = dfl_avg_Tk(components);
    avg_val[index_pk] = dfl_avg_Pk(
        components, GAS_CONSTANT / avg_val[index_mol], avg_val[index_tk]);
    avg_val[index_accent] = dfl_avg_acentric(components);
#else
    // todo: вообще-то нужно разобраться с каталогом(выбором)
    //   этих функций ч/з конфигурацию программы
    avg_val[index_pk] = 0.0;
    avg_val[index_tk] = lee_avg_Tk(components);
    avg_val[index_vk] = ch_pr_avg_Vk(components);
    avg_val[index_zk] = 0.0;
    avg_val[index_accent] = 0.0;

#endif  // UNDEFINED_DEFINE
  }
  return avg_val;
}
}  // namespace ns_avg

// GasParameters_mix
GasParameters_mix::GasParameters_mix(parameters prs,
                                     const_parameters cgp,
                                     dyn_parameters dgp,
                                     parameters_mix components)
    : GasParameters(prs, cgp, dgp), components_(components) {}

GasParameters_mix::~GasParameters_mix() {}

GasParameters_mix_dyn::GasParameters_mix_dyn(parameters prs,
                                             const_parameters cgp,
                                             dyn_parameters dgp,
                                             parameters_mix components,
                                             modelGeneral* mg)
    : GasParameters_mix(prs, cgp, dgp, components),
      prev_vpte_(prs),
      model_(mg) {}

GasParameters_mix_dyn* GasParameters_mix_dyn::Init(gas_params_input gpi,
                                                   modelGeneral* mg) {
  GasParameters_mix_dyn* mix = nullptr;
  if (!gpi.const_dyn.components->empty() && (mg != nullptr)) {
    // рассчитать средние критические параметры смеси
    //   как арифметическое среднее её компонентов
    // n.b. это скорее неправильный подход, переделать
    std::array<double, 6> avr_vals = ns_avg::get_average_params(
        *gpi.const_dyn.components, mg->GetModelShortInfo());
    // init gasmix const_parameters
    auto tmp_cgp = std::unique_ptr<const_parameters>(const_parameters::Init(
        GAS_TYPE_MIX, avr_vals[ns_avg::index_vk], avr_vals[ns_avg::index_pk],
        avr_vals[ns_avg::index_tk], avr_vals[ns_avg::index_zk],
        avr_vals[ns_avg::index_mol], avr_vals[ns_avg::index_accent]));
    if (tmp_cgp.get()) {
      mix = new GasParameters_mix_dyn({0.0, gpi.p, gpi.t}, *tmp_cgp,
                                      dyn_parameters(),
                                      *gpi.const_dyn.components, mg);
    } else {
      Logging::Append(ERROR_PAIR_DEFAULT(ERROR_CALC_GAS_P_ST));
      Logging::Append(io_loglvl::debug_logs,
                      "Пустой список компонентов газовой смеси ");
    }
  } else {
    Logging::Append(ERROR_PAIR_DEFAULT(ERROR_INIT_NULLP_ST));
    Logging::Append(io_loglvl::debug_logs,
                    "Пустой список компонентов газовой смеси "
                    "или не инициализирована модель");
  }
  return mix;
}

void GasParameters_mix_dyn::InitDynamicParams() {
  /*
  dyn_setup setup = DYNAMIC_SETUP_MASK;
  double volume = 0.0;
  std::vector<std::pair<double, dyn_parameters>> dgp_cpt;
  model_->update_dyn_params(dgp_cpt.back().second,
    { volume, gpi.p, gpi.t}, x.second.first);
  std::array<double, 3> dgp_tmp = {0.0, 0.0, 0.0};
  for (auto const &x : dgp_cpt) {
    dgp_tmp[0] += x.first * x.second.heat_cap_vol;
    dgp_tmp[1] += x.first * x.second.heat_cap_pres;
    dgp_tmp[2] += x.first * x.second.internal_energy;
    setup &= x.second.setup;
  }
  tmp_dgp = std::unique_ptr<dyn_parameters>(dyn_parameters::Init(setup,
      dgp_tmp[0], dgp_tmp[1], dgp_tmp[2], {volume, gpi.p, gpi.t}));
  */
  status_ = STATUS_OK;
}

std::unique_ptr<const_parameters> GasParameters_mix_dyn::GetAverageParams(
    parameters_mix& components,
    const model_str& mi) {
  std::unique_ptr<const_parameters> tmp_cgp = nullptr;
  if (!components.empty()) {
    std::array<double, 6> avr_vals = ns_avg::get_average_params(components, mi);
    // init gasmix const_parameters
    if (!(tmp_cgp = std::unique_ptr<const_parameters>(const_parameters::Init(
              GAS_TYPE_MIX, avr_vals[ns_avg::index_vk],
              avr_vals[ns_avg::index_pk], avr_vals[ns_avg::index_tk],
              avr_vals[ns_avg::index_zk], avr_vals[ns_avg::index_mol],
              avr_vals[ns_avg::index_accent]))))
      Logging::Append(ERROR_CALC_GAS_P_ST,
                      "Расчёт средних параметров для газовой смеси");
  } else {
    Logging::Append(ERROR_INIT_NULLP_ST,
                    "Инициализация газовой смеси компонентов нет");
  }
  return tmp_cgp;
}

const parameters_mix& GasParameters_mix_dyn::GetComponents() const {
  return components_;
}

void GasParameters_mix_dyn::csetParameters(double v,
                                           double p,
                                           double t,
                                           state_phase sp) {
  std::swap(prev_vpte_, vpte_);
  vpte_.volume = v;
  vpte_.pressure = p;
  vpte_.temperature = t;
  sph_ = sp;
  /*
  assert(0 && "удалить обновление динамических параметров или "
      "сделать правильно - в зависимости от используемой модели. "
      "не таким методом - не для каждлго компонента отдельно");
  // todo: здесь не совсем правильно -
  //   модель выставлена для смеси(скорее всего),
  //   а используется для компонентов отдельно!
  for (auto &x : components_)
    model_->update_dyn_params(x.second.second, vpte_);
  dyn_params_.heat_cap_vol  = 0.0;
  dyn_params_.heat_cap_pres = 0.0;
  dyn_params_.internal_energy = 0.0;
  dyn_params_.beta_kr = 0.0;
  dyn_params_.parm = vpte_;
  for (auto const &x : components_) {
    dyn_params_.heat_cap_vol  += x.first * x.second.second.heat_cap_vol;
    dyn_params_.heat_cap_pres += x.first * x.second.second.heat_cap_pres;
    dyn_params_.internal_energy +=
        x.first * x.second.second.internal_energy;
  }
  */
}
