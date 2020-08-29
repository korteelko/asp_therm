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

#include "gas_description_dynamic.h"
#include "Logging.h"
#include "models_math.h"

#include <numeric>

#include <assert.h>


/* функции, параметры, обозначения из книги
 *   Р. Рида, Дж. Праусница, Т. Шервуда
 *   "Свойства газов и жидкости" */
static model_str redlich_kwong_soave_mi(rg_model_id(rg_model_t::REDLICH_KWONG,
    MODEL_RK_SUBTYPE_SOAVE), 1, 0, "Модель Редлиха-Квонга модификации Соаве");

static model_priority rks_priority(DEF_PRIOR_RKS);

// Рид, Праусниц, Шервуд
//   Свойства жидкости и газов
//   стр 79
// не прописаны: н-Ундекан, о-Ксилол, м-Ксилол, п-Ксилол
static binary_coef_map SRK_coefs = binary_coef_map {
  {gas_pair(CH(CARBON_DIOXIDE), CH(METHANE)), 0.12},
  {gas_pair(CH(CARBON_DIOXIDE), CH(ETHANE)), 0.15},
  {gas_pair(CH(CARBON_DIOXIDE), CH(PROPANE)), 0.15},
  {gas_pair(CH(CARBON_DIOXIDE), CH(ISO_BUTANE)), 0.15},
  {gas_pair(CH(CARBON_DIOXIDE), CH(N_BUTANE)), 0.15},
  {gas_pair(CH(CARBON_DIOXIDE), CH(ISO_PENTANE)), 0.15},
  {gas_pair(CH(CARBON_DIOXIDE), CH(N_PENTANE)), 0.15},
  {gas_pair(CH(CARBON_DIOXIDE), CH(HEXANE)), 0.15},
  {gas_pair(CH(CARBON_DIOXIDE), CH(HEPTANE)), 0.15},
  {gas_pair(CH(CARBON_DIOXIDE), CH(OCTANE)), 0.15},
  {gas_pair(CH(CARBON_DIOXIDE), CH(NONANE)), 0.15},
  {gas_pair(CH(CARBON_DIOXIDE), CH(DECANE)), 0.15},

  {gas_pair(CH(HYDROGEN_SULFIDE), CH(METHANE)), 0.08},
  {gas_pair(CH(HYDROGEN_SULFIDE), CH(ETHANE)), 0.07},
  {gas_pair(CH(HYDROGEN_SULFIDE), CH(PROPANE)), 0.07},
  {gas_pair(CH(HYDROGEN_SULFIDE), CH(ISO_BUTANE)), 0.06},
  {gas_pair(CH(HYDROGEN_SULFIDE), CH(N_BUTANE)), 0.06},
  {gas_pair(CH(HYDROGEN_SULFIDE), CH(ISO_PENTANE)), 0.06},
  {gas_pair(CH(HYDROGEN_SULFIDE), CH(N_PENTANE)), 0.06},
  {gas_pair(CH(HYDROGEN_SULFIDE), CH(HEXANE)), 0.05},
  {gas_pair(CH(HYDROGEN_SULFIDE), CH(HEPTANE)), 0.04},
  {gas_pair(CH(HYDROGEN_SULFIDE), CH(OCTANE)), 0.04},
  {gas_pair(CH(HYDROGEN_SULFIDE), CH(NONANE)), 0.03},
  {gas_pair(CH(HYDROGEN_SULFIDE), CH(DECANE)), 0.03},
  {gas_pair(CH(HYDROGEN_SULFIDE), CH(CARBON_DIOXIDE)), 0.12},

  {gas_pair(CH(NITROGEN), CH(METHANE)), 0.02},
  {gas_pair(CH(NITROGEN), CH(ETHANE)), 0.06},
  {gas_pair(CH(NITROGEN), CH(PROPANE)), 0.08},
  {gas_pair(CH(NITROGEN), CH(ISO_BUTANE)), 0.08},
  {gas_pair(CH(NITROGEN), CH(N_BUTANE)), 0.08},
  {gas_pair(CH(NITROGEN), CH(ISO_PENTANE)), 0.08},
  {gas_pair(CH(NITROGEN), CH(N_PENTANE)), 0.08},
  {gas_pair(CH(NITROGEN), CH(HEXANE)), 0.08},
  {gas_pair(CH(NITROGEN), CH(HEPTANE)), 0.08},
  {gas_pair(CH(NITROGEN), CH(OCTANE)), 0.08},
  {gas_pair(CH(NITROGEN), CH(NONANE)), 0.08},
  {gas_pair(CH(NITROGEN), CH(DECANE)), 0.08},
  {gas_pair(CH(NITROGEN), CH(CARBON_DIOXIDE)), 0.0},

  {gas_pair(CH(CARBON_MONOXIDE), CH(METHANE)), -0.02},
  {gas_pair(CH(CARBON_MONOXIDE), CH(CARBON_DIOXIDE)), -0.04},

  {gas_pair(CH(UNDEFINED), CH(UNDEFINED)), 0.000},
#if defined(ISO_20765)
  {gas_pair(CH(CARBON_DIOXIDE), CH(ETHYLEN)), 0.15},
  {gas_pair(CH(CARBON_DIOXIDE), CH(PROPYLEN)), 0.08},
  {gas_pair(CH(CARBON_DIOXIDE), CH(CYCLOHEXANE)), 0.15},
  {gas_pair(CH(CARBON_DIOXIDE), CH(MCYCLOHEXANE)), 0.15},
  {gas_pair(CH(CARBON_DIOXIDE), CH(BENZENE)), 0.15},
  {gas_pair(CH(CARBON_DIOXIDE), CH(TOLUENE)), 0.15},
  {gas_pair(CH(CARBON_DIOXIDE), CH(ETHYLBENZENE)), 0.15},

  {gas_pair(CH(HYDROGEN_SULFIDE), CH(ETHYLEN)), 0.07},
  {gas_pair(CH(HYDROGEN_SULFIDE), CH(PROPYLEN)), 0.07},
  {gas_pair(CH(HYDROGEN_SULFIDE), CH(CYCLOHEXANE)), 0.03},
  {gas_pair(CH(HYDROGEN_SULFIDE), CH(MCYCLOHEXANE)), 0.03},
  {gas_pair(CH(HYDROGEN_SULFIDE), CH(BENZENE)), 0.03},
  {gas_pair(CH(HYDROGEN_SULFIDE), CH(TOLUENE)), 0.03},
  {gas_pair(CH(HYDROGEN_SULFIDE), CH(ETHYLBENZENE)), 0.03},

  {gas_pair(CH(NITROGEN), CH(ETHYLEN)), 0.04},
  {gas_pair(CH(NITROGEN), CH(PROPYLEN)), 0.06},
  {gas_pair(CH(NITROGEN), CH(CYCLOHEXANE)), 0.08},
  {gas_pair(CH(NITROGEN), CH(MCYCLOHEXANE)), 0.08},
  {gas_pair(CH(NITROGEN), CH(BENZENE)), 0.08},
  {gas_pair(CH(NITROGEN), CH(TOLUENE)), 0.08},
  {gas_pair(CH(NITROGEN), CH(ETHYLBENZENE)), 0.08},
#endif  // ISO_20765
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

static double calculate_fw(double w) {
  return 0.480 + 1.574*w - 0.176*w*w;
}

static double calculate_ac(const const_parameters &cp) {
  return 0.42747 * std::pow(cp.R, 2.0) *
      std::pow(cp.T_K, 2.0) / cp.P_K;
}

static double calculate_b(const const_parameters &cp) {
  return 0.08664 * cp.R * cp.T_K / cp.P_K;
}

#ifdef RPS_FUNCTIONS
double Redlich_Kwong_Soave::calculate_F(double t,
    const const_parameters &cp) {
  return Redlich_Kwong_Soave::calculate_F(
      t, calculate_fw(cp.acentricfactor), cp);
}

double Redlich_Kwong_Soave::calculate_F(double t, double wf,
    const const_parameters &cp) {
  double m = 1.0 + wf * (1.0 - sqrt(t / cp.T_K));
  return m * m * cp.T_K / t;
}
#endif  // RPS_FUNCTIONS

void Redlich_Kwong_Soave::update_coef_a(double t) {
  if (HasGasMixMark(gm_)) {
    update_gasmix_coef_a(t);
  } else {
    model_coef_a_ = coef_ac_ *
        const_rks_vals_[0].calculate_a(t / parameters_->cgetT_K());
  }
}

void Redlich_Kwong_Soave::update_gasmix_coef_a(double t) {
  /* todo: redo this ಥʖ̯ಥ(sadness cry)
   *   indexes and iterators */
  GasParameters_mix_dyn *gp =
      dynamic_cast<GasParameters_mix_dyn *>(parameters_.get());
  if (!gp) {
    error_.SetError(ERROR_CALC_MIX_ST, "dyn_cast");
    error_.LogIt(io_loglvl::debug_logs);
    status_ = STATUS_HAVE_ERROR;
    return;
  }
  const parameters_mix &prs = gp->GetComponents();
  std::vector<double> a(prs.size(), 0.0);
  int i = 0, j = 0;
  auto prs_it = prs.begin();
  model_coef_a_ = 0.0;
  for (const auto &x: const_rks_vals_) {
    a[i++] = x.calculate_a(t / prs_it->second.first.T_K);
    prs_it++;
  }
  i = 0;
  for (const auto &x: prs) {
    j = 0;
    for (const auto &y: prs) {
      model_coef_a_ += (1.0 - get_binary_associate_coef_SRK(
          x.second.first.gas_name, y.second.first.gas_name)) *
          x.first * y.first * sqrt(a[i]*a[j]);
      ++j;
    }
    ++i;
  }
}

void Redlich_Kwong_Soave::set_pure_gas_vals(const const_parameters &cp) {
  coef_ac_ = 0.42747 * std::pow(cp.R, 2.0) *
      std::pow(cp.T_K, 2.0) / cp.P_K;
  model_coef_b_ = 0.08664 * cp.R * cp.T_K / cp.P_K;
  const_rks_vals_ = std::vector<const_rks_val>();
  const_rks_vals_.push_back(
      const_rks_val(calculate_ac(cp), calculate_fw(cp.acentricfactor)));
}

void Redlich_Kwong_Soave::set_rks_const_vals(
    const parameters_mix *components) {
  /* расчитать константные части функций коэффициентов */
  //   const_rks_vals_rps_.set_vals(components);
  const_rks_vals_ = std::vector<const_rks_val>();
  std::transform(components->begin(), components->end(),
      std::back_insert_iterator<std::vector<const_rks_val>>(const_rks_vals_),
      [](const std::pair<const double, const_dyn_parameters> &p) {
          return const_rks_val(calculate_ac(p.second.first),
          calculate_fw(p.second.first.acentricfactor));});
}

void Redlich_Kwong_Soave::set_gasmix_model_coefs(const model_input &mi) {
  const parameters_mix *pm_p = mi.gpi.const_dyn.components;
  model_coef_b_ = 0.0;
  for (const auto &x : *pm_p)
    model_coef_b_ += x.first * calculate_b(x.second.first);
}

#ifdef RPS_FUNCTIONS
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
    fi = calculate_F(t, const_rks_vals_rps_.fw_i[i], x.second.first);
    for (const auto &y : *pm_p) {
      fj = calculate_F(t, const_rks_vals_rps_.fw_i[j], y.second.first);
      double tp = const_rks_vals_rps_.fsqrt_tp_ij[i][j];
      fm += tp * sqrt(fi * fj);
      ++j;
    }
    ++i;
  }
  fm /= const_rks_vals_rps_.ftp_sum;
  if (!is_equal(const_rks_vals_rps_.ftp_sum, 0.0, FLOAT_ACCURACY)) {
    fm /= const_rks_vals_rps_.ftp_sum;
  } else {
    status_ = STATUS_HAVE_ERROR;
    error_.SetError(ERROR_INIT_ZERO_ST,
        "Инициализация газовой смеси для уравенея Соаве-Редлиха-Квонга");
  }
  model_coef_b_ = result_b_coef;
}
#endif  // RPS_FUNCTIONS

Redlich_Kwong_Soave::Redlich_Kwong_Soave(const model_input &mi)
  : modelGeneral(mi.ms, mi.gm, mi.bp) {
  if (HasGasMixMark(gm_)) {
    /* газовая смесь: */
    set_rks_const_vals(mi.gpi.const_dyn.components);
    // const_rks_vals_rps_.set_vals(mi.gpi.const_dyn.components);
    /* установить коэфициенты модели для смеси */
    set_gasmix_model_coefs(mi);
    // подумоть про этот подход
    // gasmix_model_coefs_rps(mi);
    /* рассчитать усреднённые const параметры(Pk, Tk, Vk),
     *   инициализровать начальные параметры смеси - v, cp, cv, u... */
    set_gasparameters(mi.gpi, this);
  } else {
    /* установить коэфициенты модели для смеси */
    set_pure_gas_vals(*mi.gpi.const_dyn.cdp.cgp);
    /* чистый газ: */
    /* задать параметры газа, инициализровать
     *   начальные параметры смеси - v, cp, cv, u...*/
    set_gasparameters(mi.gpi, this);
  }
  if (!error_.GetErrorCode()) {
    if (mi.mpri.IsSpecified()) {
      priority_ = mi.mpri;
    } else {
      priority_ = rks_priority;
    }
    update_gasmix_coef_a(mi.gpi.t);
    if (parameters_->cgetDynSetup() & DYNAMIC_ENTALPHY)
      set_enthalpy();
    SetVolume(mi.gpi.p, mi.gpi.t);
    status_ = STATUS_OK;
  }
}

Redlich_Kwong_Soave *Redlich_Kwong_Soave::Init(const model_input &mi) {
  try {
    check_input(mi);
  } catch (const model_init_exception &e) {
    Logging::Append(e.what());
    return nullptr;
  }
  Redlich_Kwong_Soave *rk = new Redlich_Kwong_Soave(mi);
  if (rk)
    if (rk->parameters_ == nullptr) {
      delete rk;
      rk = nullptr;
    }
  return rk;
}

model_str Redlich_Kwong_Soave::GetModelShortInfo(const rg_model_id &) {
  return redlich_kwong_soave_mi;
}

model_str Redlich_Kwong_Soave::GetModelShortInfo() const {
  return redlich_kwong_soave_mi;
}

void Redlich_Kwong_Soave::update_dyn_params(dyn_parameters &prev_state,
    const parameters new_state) {
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

// todo: replace
bool Redlich_Kwong_Soave::IsValid() const {
  // assert(0);
  return (parameters_->cgetPressure()/parameters_->cgetP_K() <
      0.5*parameters_->cgetTemperature()/parameters_->cgetT_K());
}

bool Redlich_Kwong_Soave::IsValid(parameters prs) const {
  // assert(0);
  return (prs.pressure/parameters_->cgetP_K() <
      0.5*prs.pressure/parameters_->cgetT_K());
}

void Redlich_Kwong_Soave::SetVolume(double p, double t) {
  set_parameters(GetVolume(p, t), p, t);
}

void Redlich_Kwong_Soave::SetPressure(double v, double t) {
  set_parameters(v, GetPressure(v, t), t);
}

double Redlich_Kwong_Soave::GetVolume(double p, double t) {
  update_coef_a(t);
  if (!is_above0(p, t)) {
    error_.SetError(ERROR_PAIR_DEFAULT(ERROR_CALC_MODEL_ST));
    return 0.0;
  }
  std::vector<double> coef {
      1.0,
      -parameters_->cgetR() * t / p,
      model_coef_a_/p - parameters_->cgetR()*t*model_coef_b_/p -
          model_coef_b_*model_coef_b_,
      -model_coef_a_*model_coef_b_/p,
      0.0, 0.0, 0.0
  };
  // Следующая функция заведомо получает валидные
  //   данные,  соответственно должна что-то вернуть
  //   Не будем перегружать код лишними проверками
  int roots_count;
  CardanoMethod_roots_count(&coef[0], &coef[4], &roots_count);
#ifdef _DEBUG
  if (!is_above0(coef[4])) {
    error_.SetError(ERROR_PAIR_DEFAULT(ERROR_CALC_MODEL_ST));
    error_.LogIt();
    return 0.0;
  }
#endif  // _DEBUG
  return coef[4];
}

double Redlich_Kwong_Soave::GetPressure(double v, double t) {
  update_coef_a(t);
  if (!is_above0(v, t)) {
    error_.SetError(ERROR_PAIR_DEFAULT(ERROR_CALC_MODEL_ST));
    status_ = STATUS_HAVE_ERROR;
    return 0.0;
  }
  const double temp = parameters_->cgetR() * t / (v - model_coef_b_) -
      model_coef_a_ / (v * (v + model_coef_b_));
  return temp;
}

Redlich_Kwong_Soave::const_rks_val::const_rks_val(double ac, double fw)
  : ac(ac), fw(fw) {}

double Redlich_Kwong_Soave::const_rks_val::calculate_a(double tr) const {
  return ac * pow(1.0 + fw * (1.0 - sqrt(tr)), 2.0);
}

#ifdef RPS_FUNCTIONS
void Redlich_Kwong_Soave::const_rks_vals_rps::set_vals(
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
#endif  // RPS_FUNCTIONS
