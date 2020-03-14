/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#include "model_redlich_kwong_soave.h"

#include "common.h"
#include "gas_description_dynamic.h"
#include "ErrorWrap.h"
#include "models_math.h"

#include <numeric>

#include <assert.h>


/* функции, параметры, обозначения из книги
 *   Р. Рида, Дж. Праусница, Т. Шервуда
 *   "Свойства газов и жидкости" */
static model_str redlich_kwong_soave_mi(rg_model_t::REDLICH_KWONG,
    MODEL_RK_SUBTYPE_SOAVE, 1, 0, "Модель Редлиха-Квонга модификации Соаве");

// Рид, Праусниц, Шервуд
//   Свойства жидкости и газов
//   стр 79
// не прописаны: н-Ундекан, о-Ксилол, м-Ксилол, п-Ксилол
static binary_coef_map SRK_coefs = binary_coef_map {
  {gas_pair(GAS_TYPE_CARBON_DIOXIDE, GAS_TYPE_METHANE), 0.12},
  {gas_pair(GAS_TYPE_CARBON_DIOXIDE, GAS_TYPE_ETHYLEN), 0.15},
  {gas_pair(GAS_TYPE_CARBON_DIOXIDE, GAS_TYPE_ETHANE), 0.15},
  {gas_pair(GAS_TYPE_CARBON_DIOXIDE, GAS_TYPE_PROPYLEN), 0.08},
  {gas_pair(GAS_TYPE_CARBON_DIOXIDE, GAS_TYPE_PROPANE), 0.15},
  {gas_pair(GAS_TYPE_CARBON_DIOXIDE, GAS_TYPE_ISO_BUTANE), 0.15},
  {gas_pair(GAS_TYPE_CARBON_DIOXIDE, GAS_TYPE_N_BUTANE), 0.15},
  {gas_pair(GAS_TYPE_CARBON_DIOXIDE, GAS_TYPE_ISO_PENTANE), 0.15},
  {gas_pair(GAS_TYPE_CARBON_DIOXIDE, GAS_TYPE_N_PENTANE), 0.15},
  {gas_pair(GAS_TYPE_CARBON_DIOXIDE, GAS_TYPE_HEXANE), 0.15},
  {gas_pair(GAS_TYPE_CARBON_DIOXIDE, GAS_TYPE_HEPTANE), 0.15},
  {gas_pair(GAS_TYPE_CARBON_DIOXIDE, GAS_TYPE_OCTANE), 0.15},
  {gas_pair(GAS_TYPE_CARBON_DIOXIDE, GAS_TYPE_NONANE), 0.15},
  {gas_pair(GAS_TYPE_CARBON_DIOXIDE, GAS_TYPE_DECANE), 0.15},
  {gas_pair(GAS_TYPE_CARBON_DIOXIDE, GAS_TYPE_CYCLOHEXANE), 0.15},
  {gas_pair(GAS_TYPE_CARBON_DIOXIDE, GAS_TYPE_MCYCLOHEXANE), 0.15},
  {gas_pair(GAS_TYPE_CARBON_DIOXIDE, GAS_TYPE_BENZENE), 0.15},
  {gas_pair(GAS_TYPE_CARBON_DIOXIDE, GAS_TYPE_TOLUENE), 0.15},
  {gas_pair(GAS_TYPE_CARBON_DIOXIDE, GAS_TYPE_ETHYLBENZENE), 0.15},

  {gas_pair(GAS_TYPE_HYDROGEN_SULFIDE, GAS_TYPE_METHANE), 0.08},
  {gas_pair(GAS_TYPE_HYDROGEN_SULFIDE, GAS_TYPE_ETHYLEN), 0.07},
  {gas_pair(GAS_TYPE_HYDROGEN_SULFIDE, GAS_TYPE_ETHANE), 0.07},
  {gas_pair(GAS_TYPE_HYDROGEN_SULFIDE, GAS_TYPE_PROPYLEN), 0.07},
  {gas_pair(GAS_TYPE_HYDROGEN_SULFIDE, GAS_TYPE_PROPANE), 0.07},
  {gas_pair(GAS_TYPE_HYDROGEN_SULFIDE, GAS_TYPE_ISO_BUTANE), 0.06},
  {gas_pair(GAS_TYPE_HYDROGEN_SULFIDE, GAS_TYPE_N_BUTANE), 0.06},
  {gas_pair(GAS_TYPE_HYDROGEN_SULFIDE, GAS_TYPE_ISO_PENTANE), 0.06},
  {gas_pair(GAS_TYPE_HYDROGEN_SULFIDE, GAS_TYPE_N_PENTANE), 0.06},
  {gas_pair(GAS_TYPE_HYDROGEN_SULFIDE, GAS_TYPE_HEXANE), 0.05},
  {gas_pair(GAS_TYPE_HYDROGEN_SULFIDE, GAS_TYPE_HEPTANE), 0.04},
  {gas_pair(GAS_TYPE_HYDROGEN_SULFIDE, GAS_TYPE_OCTANE), 0.04},
  {gas_pair(GAS_TYPE_HYDROGEN_SULFIDE, GAS_TYPE_NONANE), 0.03},
  {gas_pair(GAS_TYPE_HYDROGEN_SULFIDE, GAS_TYPE_DECANE), 0.03},
  {gas_pair(GAS_TYPE_HYDROGEN_SULFIDE, GAS_TYPE_CARBON_DIOXIDE), 0.12},
  {gas_pair(GAS_TYPE_HYDROGEN_SULFIDE, GAS_TYPE_CYCLOHEXANE), 0.03},
  {gas_pair(GAS_TYPE_HYDROGEN_SULFIDE, GAS_TYPE_MCYCLOHEXANE), 0.03},
  {gas_pair(GAS_TYPE_HYDROGEN_SULFIDE, GAS_TYPE_BENZENE), 0.03},
  {gas_pair(GAS_TYPE_HYDROGEN_SULFIDE, GAS_TYPE_TOLUENE), 0.03},
  {gas_pair(GAS_TYPE_HYDROGEN_SULFIDE, GAS_TYPE_ETHYLBENZENE), 0.03},

  {gas_pair(GAS_TYPE_NITROGEN, GAS_TYPE_METHANE), 0.02},
  {gas_pair(GAS_TYPE_NITROGEN, GAS_TYPE_ETHYLEN), 0.04},
  {gas_pair(GAS_TYPE_NITROGEN, GAS_TYPE_ETHANE), 0.06},
  {gas_pair(GAS_TYPE_NITROGEN, GAS_TYPE_PROPYLEN), 0.06},
  {gas_pair(GAS_TYPE_NITROGEN, GAS_TYPE_PROPANE), 0.08},
  {gas_pair(GAS_TYPE_NITROGEN, GAS_TYPE_ISO_BUTANE), 0.08},
  {gas_pair(GAS_TYPE_NITROGEN, GAS_TYPE_N_BUTANE), 0.08},
  {gas_pair(GAS_TYPE_NITROGEN, GAS_TYPE_ISO_PENTANE), 0.08},
  {gas_pair(GAS_TYPE_NITROGEN, GAS_TYPE_N_PENTANE), 0.08},
  {gas_pair(GAS_TYPE_NITROGEN, GAS_TYPE_HEXANE), 0.08},
  {gas_pair(GAS_TYPE_NITROGEN, GAS_TYPE_HEPTANE), 0.08},
  {gas_pair(GAS_TYPE_NITROGEN, GAS_TYPE_OCTANE), 0.08},
  {gas_pair(GAS_TYPE_NITROGEN, GAS_TYPE_NONANE), 0.08},
  {gas_pair(GAS_TYPE_NITROGEN, GAS_TYPE_DECANE), 0.08},
  {gas_pair(GAS_TYPE_NITROGEN, GAS_TYPE_CARBON_DIOXIDE), 0.0},
  {gas_pair(GAS_TYPE_NITROGEN, GAS_TYPE_CYCLOHEXANE), 0.08},
  {gas_pair(GAS_TYPE_NITROGEN, GAS_TYPE_MCYCLOHEXANE), 0.08},
  {gas_pair(GAS_TYPE_NITROGEN, GAS_TYPE_BENZENE), 0.08},
  {gas_pair(GAS_TYPE_NITROGEN, GAS_TYPE_TOLUENE), 0.08},
  {gas_pair(GAS_TYPE_NITROGEN, GAS_TYPE_ETHYLBENZENE), 0.08},

  {gas_pair(GAS_TYPE_CARBON_MONOXIDE, GAS_TYPE_METHANE), -0.02},
  {gas_pair(GAS_TYPE_CARBON_MONOXIDE, GAS_TYPE_CARBON_DIOXIDE), -0.04},

  {gas_pair(GAS_TYPE_UNDEFINED, GAS_TYPE_UNDEFINED), 0.000}
};

// todo: у Пенга-Робинсона тоже самое - наверное неплохо
//   смотрелся бы функтор вместо этой функции, к тому же
//   нужно прописать на неё тесты
static double get_binary_associate_coef_SRK(gas_t i, gas_t j) {
  auto it = SRK_coefs.find(gas_pair(i, j));
  if (it != SRK_coefs.end())
    return it->second;
  return 0.0;
}

double Redlich_Kwong_Soave::calculate_F(double t,
    const const_parameters &cp) {
  return Redlich_Kwong_Soave::calculate_F(t, 0.48 + 1.574 * cp.acentricfactor -
      0.176 * cp.acentricfactor * cp.acentricfactor, cp);
}

double Redlich_Kwong_Soave::calculate_F(double t, double wf,
    const const_parameters &cp) {
  double m = 1.0 + wf * (1.0 - sqrt(t / cp.T_K));
  return m * m * cp.T_K / t;
}

std::pair<double, double> Redlich_Kwong_Soave::calculate_ab_coefs(
    const const_parameters &cp) {
  double a = 0.42747 * std::pow(cp.R, 2.0) *
      std::pow(cp.T_K, 2.0) / cp.P_K;
  double  b = 0.08664 * cp.R * cp.T_K / cp.P_K;
  return {a, b};
}

void Redlich_Kwong_Soave::set_model_coef() {
  model_coef_a_ = 0.42747 * std::pow(parameters_->cgetR(), 2.0) *
      std::pow(parameters_->cgetT_K(), 2.0) / parameters_->cgetP_K();
  model_coef_b_ = 0.08664*parameters_->cgetR()*parameters_->cgetT_K() /
      parameters_->cgetP_K();
}

void Redlich_Kwong_Soave::set_model_coef(
    const const_parameters &cp) {
  model_coef_a_ = 0.42747 * std::pow(cp.R, 2.0) *
      std::pow(cp.T_K, 2.0) / cp.P_K;
  model_coef_b_ = 0.08664 * cp.R * cp.T_K / cp.P_K;
}

void Redlich_Kwong_Soave::gasmix_model_coefs(const model_input &mi) {
  if (mi.gpi.const_dyn.components->size() == 1)
    return;
  const parameters_mix *pm_p = mi.gpi.const_dyn.components;
  double result_a_coef = 0.0,
         result_b_coef = 0.0;
  std::vector<std::pair<double, double>> ab;
  for (const auto &x: *mi.gpi.const_dyn.components)
    ab.push_back(Redlich_Kwong_Soave::calculate_ab_coefs(x.second.first));
  int i = 0, j = 0;
  for (const auto &x : *pm_p) {
    result_b_coef += x.first * ab[i].second;
    for (const auto &y : *pm_p) {
      result_a_coef += (1.0 - get_binary_associate_coef_SRK(
          x.second.first.gas_name, y.second.first.gas_name)) *
          x.first * y.first * sqrt(ab[i].second * ab[j].second);
      ++j;
    }
    ++i;
  }
  model_coef_a_ = result_a_coef;
  model_coef_b_ = result_b_coef;
}

// todo: test it
void Redlich_Kwong_Soave::gasmix_model_coefs_rps(const model_input &mi) {
  assert(0 && "redo|update");
  if (mi.gpi.const_dyn.components->size() == 1)
    return;
  const parameters_mix *pm_p = mi.gpi.const_dyn.components;
  double t = mi.gpi.t;
  double result_a_coef = 0.0,
         result_b_coef = 0.0;
  double fm = 0.0, fi, fj;
  int i = 0, j = 0;
  for (const auto &x : *pm_p) {
    set_model_coef(x.second.first);
    result_b_coef += x.first * model_coef_b_;
    fi = calculate_F(t, const_rks_vals_.fw_i[i], x.second.first);
    for (const auto &y : *pm_p) {
      fj = calculate_F(t, const_rks_vals_.fw_i[j], y.second.first);
      double tp = const_rks_vals_.fsqrt_tp_ij[i][j];
      fm += tp * sqrt(fi * fj);
      ++j;
    }
    ++i;
  }
  fm /= const_rks_vals_.ftp_sum;
  if (!is_equal(const_rks_vals_.ftp_sum, 0.0, FLOAT_ACCURACY)) {
    fm /= const_rks_vals_.ftp_sum;
  } else {
    status_ = STATUS_HAVE_ERROR;
    error_.SetError(ERROR_GAS_MIX | ERROR_INIT_ZERO_ST,
        "Инициализация газовой смеси для уравенея Соаве-Редлиха-Квонга");
  }
  model_coef_b_ = result_b_coef;
}

Redlich_Kwong_Soave::Redlich_Kwong_Soave(const model_input &mi)
  : modelGeneral(mi.calc_config, mi.gm, mi.bp) {
  if (HasGasMixMark(gm_)) {
    /* газовая смесь: */
    /* расчитать константные части функций коэффициентов */
    const_rks_vals_.set_vals(mi.gpi.const_dyn.components);
    /* установить коэфициенты модели для смеси */
    gasmix_model_coefs(mi);
    // подумоть про этот подход
    // gasmix_model_coefs_rps(mi);
    /* рассчитать усреднённые const параметры(Pk, Tk, Vk),
     *   инициализровать начальные параметры смеси - v, cp, cv, u... */
    set_gasparameters(mi.gpi, this);
  } else {
    /* чистый газ: */
    /* задать параметры газа, инициализровать
     *   начальные параметры смеси - v, cp, cv, u...*/
    set_gasparameters(mi.gpi, this);
    /* установить коэфициенты модели для смеси */
    set_model_coef();
  }
  if (!error_.GetErrorCode()) {
    if (parameters_->cgetDynSetup() & DYNAMIC_ENTALPHY)
      set_enthalpy();
    SetVolume(mi.gpi.p, mi.gpi.t);
  }
}

Redlich_Kwong_Soave *Redlich_Kwong_Soave::Init(const model_input &mi) {
  if (check_input(mi))
    return nullptr;
  Redlich_Kwong_Soave *rk = new Redlich_Kwong_Soave(mi);
  if (rk)
    if (rk->parameters_ == nullptr) {
      delete rk;
      rk = nullptr;
    }
  return rk;
 }

model_str Redlich_Kwong_Soave::GetModelShortInfo() const {
  return redlich_kwong_soave_mi;
}

void Redlich_Kwong_Soave::update_dyn_params(dyn_parameters &prev_state,
    const parameters new_state) {
  assert(0);
  /*double du  = internal_energy_integral(new_state, prev_state.parm);
  // heat_capacity_volume addition
  double dcv = heat_capac_vol_integral(new_state, prev_state.parm);
  // cp - cv
  double dif_c = heat_capac_dif_prs_vol(new_state,
      parameters_->const_params.R);   prev_state.internal_energy += du;
  prev_state.heat_cap_vol    += dcv;
  prev_state.heat_cap_pres   = prev_state.heat_cap_vol + dif_c;
  prev_state.parm = new_state;
  prev_state.Update();*/
}

void Redlich_Kwong_Soave::update_dyn_params(dyn_parameters &prev_state,
    const parameters new_state, const const_parameters &cp) {
  /*set_model_coef(cp);
  double du  = internal_energy_integral(new_state, prev_state.parm);
  // heat_capacity_volume addition
  double dcv = heat_capac_vol_integral(new_state, prev_state.parm);
  // cp - cv
  double dif_c = heat_capac_dif_prs_vol(new_state, cp.R);
  prev_state.internal_energy += du;
  prev_state.heat_cap_vol    += dcv;
  prev_state.heat_cap_pres   = prev_state.heat_cap_vol + dif_c;
  prev_state.parm = new_state;
  prev_state.Update(); */
}

// visitor
void Redlich_Kwong_Soave::DynamicflowAccept(DerivateFunctor &df) {
  df.getFunctor(*this);
}

bool Redlich_Kwong_Soave::IsValid() const {
  assert(0);
  return (parameters_->cgetPressure()/parameters_->cgetP_K() <
      0.5*parameters_->cgetTemperature()/parameters_->cgetT_K());
}

double Redlich_Kwong_Soave::InitVolume(double p, double t,
    const const_parameters &cp) {
  assert(0);
  // set_model_coef(cp);
  // return get_volume(p, t, cp);
  return 0.0;
}

void Redlich_Kwong_Soave::SetVolume(double p, double t) {
  set_parameters(GetVolume(p, t), p, t);
}

void Redlich_Kwong_Soave::SetPressure(double v, double t) {
  set_parameters(v, GetPressure(v, t), t);
}

double Redlich_Kwong_Soave::GetVolume(double p, double t) {
  if (!is_above0(p, t)) {
    error_.SetError(ERROR_CALC_MODEL_ST);
    return 0.0;
  }
  std::vector<double> coef {
      1.0,
      -parameters_->cgetR()*t/p,
      model_coef_a_/(p*std::sqrt(t)) - parameters_->cgetR()*
          t*model_coef_b_/p - model_coef_b_*model_coef_b_,
      -model_coef_a_*model_coef_b_/(p*std::sqrt(t)),
      0.0, 0.0, 0.0
  };
  // Следующая функция заведомо получает валидные
  //   данные,  соответственно должна что-то вернуть
  //   Не будем перегружать код лишними проверками
  int roots_count;
  CardanoMethod_roots_count(&coef[0], &coef[4], &roots_count);
#ifdef _DEBUG
  if (!is_above0(coef[4])) {
    error_.SetError(ERROR_CALC_MODEL_ST);
    error_.LogIt();
    return 0.0;
  }
#endif  // _DEBUG
  return coef[4];
}

double Redlich_Kwong_Soave::GetPressure(double v, double t) {
  assert(0);
  if (!is_above0(v, t)) {
    error_.SetError(ERROR_CALC_MODEL_ST);
    return 0.0;
  }
  const double temp = parameters_->cgetR() * t / (v - model_coef_b_) -
      model_coef_a_ / (std::sqrt(t)* v *(v + model_coef_b_));
  return temp;
}

void Redlich_Kwong_Soave::const_rks_vals::set_vals(
    const parameters_mix *components) {
  ftp_sum = 0.0;
  fw_i.assign(components->size(), 0.0);
  fsqrt_tp_ij.assign(components->size(), std::vector<double>());
  auto tp_i_it = fsqrt_tp_ij.begin();
  auto fw_i_it = fw_i.begin();
  double w = 0.0;
  double k;
  const const_parameters *ci, *cj;
  for (const auto &x : *components) {
    ci = &x.second.first;
    w = ci->acentricfactor;
    ftp_sum += x.first * ci->T_K / ci->P_K;
    *fw_i_it++ = 0.480 + 1.574 * w - 0.176 * w * w;
    tp_i_it->assign(components->size(), 0.0);
    auto tp_ij_it = tp_i_it->begin();
    for (const auto &y : *components) {
      cj = &y.second.first;
      k = get_binary_associate_coef_SRK(ci->gas_name, cj->gas_name);
      *tp_ij_it++ = x.first * y.first * (1.0-k) *
          sqrt(ci->T_K * cj->T_K / ci->P_K / cj->P_K);
    }
  }
}
