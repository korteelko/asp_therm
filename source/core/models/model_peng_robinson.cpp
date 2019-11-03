#include "model_peng_robinson.h"

#include "common.h"
#include "gas_description_dynamic.h"
#include "models_errors.h"
#include "models_math.h"

#ifdef _DEBUG
#  include <iostream>
#endif  // _DEBUG
#include <vector>

#include <assert.h>

struct binary_associate_PR {
  gas_t i,
        j;
  double c;
};

static binary_associate_PR PR_coefs[] = {
  {GAS_TYPE_NITROGEN, GAS_TYPE_CARBON_DIOXIDE, 0.0},
  {GAS_TYPE_NITROGEN, GAS_TYPE_HYDROGEN_SULFIDE, 0.130},
  {GAS_TYPE_NITROGEN, GAS_TYPE_METHANE, 0.025},
  {GAS_TYPE_NITROGEN, GAS_TYPE_ETHANE, 0.010},
  {GAS_TYPE_NITROGEN, GAS_TYPE_PROPANE, 0.090},
  {GAS_TYPE_NITROGEN, GAS_TYPE_N_BUTANE, 0.095},

  {GAS_TYPE_CARBON_DIOXIDE, GAS_TYPE_HYDROGEN_SULFIDE, 0.135},
  {GAS_TYPE_CARBON_DIOXIDE, GAS_TYPE_METHANE, 0.105},
  {GAS_TYPE_CARBON_DIOXIDE, GAS_TYPE_ETHANE, 0.130},
  {GAS_TYPE_CARBON_DIOXIDE, GAS_TYPE_PROPANE, 0.125},
  {GAS_TYPE_CARBON_DIOXIDE, GAS_TYPE_N_BUTANE, 0.115},

  {GAS_TYPE_HYDROGEN_SULFIDE, GAS_TYPE_METHANE, 0.070},
  {GAS_TYPE_HYDROGEN_SULFIDE, GAS_TYPE_ETHANE, 0.085},
  {GAS_TYPE_HYDROGEN_SULFIDE, GAS_TYPE_PROPANE, 0.080},
  {GAS_TYPE_HYDROGEN_SULFIDE, GAS_TYPE_N_BUTANE, 0.075},

  {GAS_TYPE_METHANE, GAS_TYPE_ETHANE, 0.005},
  {GAS_TYPE_METHANE, GAS_TYPE_PROPANE, 0.010},
  {GAS_TYPE_METHANE, GAS_TYPE_N_BUTANE, 0.010},

  {GAS_TYPE_N_PENTANE, GAS_TYPE_NITROGEN, 0.100},
  {GAS_TYPE_N_PENTANE, GAS_TYPE_CARBON_DIOXIDE, 0.115},
  {GAS_TYPE_N_PENTANE, GAS_TYPE_HYDROGEN_SULFIDE, 0.070},
  {GAS_TYPE_N_PENTANE, GAS_TYPE_METHANE, 0.030},
  {GAS_TYPE_N_PENTANE, GAS_TYPE_ETHANE, 0.010},
  {GAS_TYPE_N_PENTANE, GAS_TYPE_PROPANE, 0.020},
  {GAS_TYPE_N_PENTANE, GAS_TYPE_N_BUTANE, 0.005},

  {GAS_TYPE_UNDEFINED, GAS_TYPE_UNDEFINED, 0.000}
};

static double get_binary_associate_coef_PR(gas_t i, gas_t j) {
  if (i == j)
    return 0.0;
  size_t bin_coef_count = sizeof(PR_coefs) / sizeof(*PR_coefs);
  size_t z = 0;
  for (; z <  bin_coef_count; ++z)
    if (PR_coefs[z].i == i)
      break;
  if (z >= bin_coef_count - 1)
    return 0.0;
  while (PR_coefs[z].i == i) {
    if (PR_coefs[z].j == j)
      return PR_coefs[z].c;
    ++z;
  }
  return 0.0;
}

static double sq2 = std::sqrt(2.0);

void Peng_Robinson::set_model_coef() {
  model_coef_a_ = 0.45724 * std::pow(parameters_->cgetR(), 2.0) *
      std::pow(parameters_->cgetT_K(), 2.0) / parameters_->cgetP_K();
  model_coef_b_ = 0.0778 * parameters_->cgetR() * parameters_->cgetT_K() /
      parameters_->cgetP_K();
  model_coef_k_ = 0.37464 + 1.54226*parameters_->cgetAcentricFactor() -
      0.26992*std::pow(parameters_->cgetAcentricFactor(), 2.0);
}

void Peng_Robinson::set_model_coef(
    const const_parameters &cp) {
  model_coef_a_ = 0.45724 * std::pow(cp.R, 2.0) *
      std::pow(cp.T_K, 2.0) / cp.P_K;
  model_coef_b_ = 0.0778 * cp.R * cp.T_K / cp.P_K;
  model_coef_k_ = 0.37464 + 1.54226 * cp.acentricfactor -
      0.26992 * std::pow(cp.acentricfactor, 2.0);
}

// #ifdef BY_PSEUDO_CRITIC
model_input Peng_Robinson::set_pseudo_critic_parameters(
    const model_input &mi) {
  // available only for GAS_MIX
  //   я ничего уже не понимаю (((((((((((((
  //     сделаю как в статье из университета Келдыша
  if (mi.gpi.const_dyn.components->size() == 1)
    return mi;
  const parameters_mix *pm_p = mi.gpi.const_dyn.components;
  double part_x = 0.0,
         part_y = 0.0;
  double k_coef = 0.0;
  double result_a_coef = 0.0,
         result_b_coef = 0.0;
  double tmp_ax_coef = 0.0;
  for (const auto &x : *pm_p) {
    set_model_coef(x.second.first);
    tmp_ax_coef = model_coef_a_;
    result_b_coef += part_x * model_coef_b_;
    part_x = x.first;
    for (const auto &y : *pm_p) {
      set_model_coef(y.second.first);
      part_y = y.first;
      k_coef = get_binary_associate_coef_PR(
          x.second.first.gas_name, y.second.first.gas_name);
      result_a_coef += part_x * part_y * (1.0 - k_coef) *
          sqrt(tmp_ax_coef * model_coef_a_);
    }
  }
  model_coef_a_ = result_a_coef;
  model_coef_b_ = result_b_coef;
  return model_input{mi.gm, mi.bp, mi.gpi};
}
// #endif  // BY_PSEUDO_CRITIC

Peng_Robinson::Peng_Robinson(const model_input &mi)
  : modelGeneral(mi.gm, mi.bp) {
#ifdef BY_PSEUDO_CRITIC
  auto mi_pc = set_pseudo_critic_parameters(mi);
  if (!set_gasparameters(mi_pc.gpi, this))
    return;
#else
  if (!set_gasparameters(mi.gpi, this))
    return;
#endif  // BY_PSEUDO_CRITIC
  set_model_coef();
  set_enthalpy();
}

Peng_Robinson *Peng_Robinson::Init(const model_input &mi) {
  reset_error();
  if (check_input(mi))
    return nullptr;
  Peng_Robinson *pr = new Peng_Robinson(mi);
  if (pr)
    if (pr->parameters_ == nullptr) {
      delete pr;
      pr = nullptr;
    }
  return pr; 
}

//  расчёт смотри в ежедневнике
double Peng_Robinson::log_pr(double v, bool is_posit) {
  return (is_posit) ? (1.0 + sq2) * model_coef_b_ + v : 
      (1.0 - sq2) * model_coef_b_ + v;
}

double Peng_Robinson::internal_energy_integral(const parameters new_state,
    const parameters old_state, const double Tk) {
  double t  = new_state.temperature,
         Tr = new_state.temperature / Tk,
         vf = new_state.volume,
         v0 = old_state.volume,
         a  = model_coef_a_,
         b  = model_coef_b_,
         k  = model_coef_k_;
  double ans = sq2 * 0.25 * a * k * t * t * 
      (-k*sqrt(Tr) + k - sqrt(Tr) + 2.0 + 1.0/k) * 
      log(log_pr(vf, false) * log_pr(v0, true) / 
          log_pr(vf, true) / log_pr(v0, false)) /
      (b * t * t);
  return ans;
}

// return cv - cv0
double Peng_Robinson::heat_capac_vol_integral(const parameters new_state,
    const parameters prev_state, const double Tk) {
  double t  = new_state.temperature,
         Tr = new_state.temperature / Tk,
         vf = new_state.volume,
         v0 = prev_state.volume,
         a  = model_coef_a_,
         b  = model_coef_b_,
         k  = model_coef_k_;
  double ans = sq2*Tk*a*k*t*t*sqrt(Tr)*(-1.0 - k) *
      log(log_pr(vf, false) * log_pr(v0, true) / 
          log_pr(vf, true) / log_pr(v0, false)) /
      (8.0 * Tk * b * t*t*t);
  return ans;
}

// return cp - cv
double Peng_Robinson::heat_capac_dif_prs_vol(const parameters new_state,
    const double Tk, double R) {
  double t  = new_state.temperature,
         Tr = new_state.temperature / Tk,
         v  = new_state.volume,
         a  = model_coef_a_,
         b  = model_coef_b_,
         k  = model_coef_k_;
  return  t * pow((R/(-b + v) + a*k*sqrt(Tr)*(k*(-sqrt(Tr) + 1.0) + 1.0) /
      (t*(-b*b + 2.0*b*v + v*v))), 2.0) / 
      (R*t / pow((-b + v), 2.0) - a*(-2.0*b - 2.0*v)*
      pow((k*(-sqrt(Tr) + 1.0) + 1.0), 2.0) / pow((-b*b + 2.0*b*v + v*v), 2.0));
}

double Peng_Robinson::get_volume(double p, double t, const const_parameters &cp) {
#ifdef GAS_MIX_VARIANT
  set_model_coef(cp);
#endif  // GAS_MIX_VARIANT
  double alf = std::pow(1.0 + model_coef_k_*(1.0 -
      t / cp.T_K), 2.0);
  std::vector<double> coef {
      1.0,
      model_coef_b_ - cp.R*t/p,
      (model_coef_a_*alf - 2.0 * model_coef_b_ *
          cp.R*t)/p - 3.0*model_coef_b_*model_coef_b_,
      std::pow(model_coef_b_, 3.0f) + (cp.R *
          t*model_coef_b_*model_coef_b_ - model_coef_a_ * alf *model_coef_b_)/p,
      0.0, 0.0, 0.0};
  CardanoMethod_HASUNIQROOT(&coef[0], &coef[4]);
#ifdef _DEBUG
  if (!is_above0(coef[4])) {
    error_ = set_error_code(ERR_CALCULATE_T | ERR_CALC_MODEL_ST);
    return 0.0;
  }
#endif
  return coef[4];
}

double Peng_Robinson::get_pressure(double v, double t,
    const const_parameters &cp) {
#ifdef GAS_MIX_VARIANT
  set_model_coef(cp);
#endif  // GAS_MIX_VARIANT
  const double a = std::pow(1.0 + model_coef_k_ * std::pow(1.0 -
      std::sqrt(t / cp.T_K), 2.0), 2.0);
  const double temp = cp.R*t / (v-model_coef_b_) -
      a * model_coef_a_ /
      (v*v+2.0*model_coef_b_*v -model_coef_b_*model_coef_b_);
  return temp;
}

void Peng_Robinson::update_dyn_params(dyn_parameters &prev_state,
    const parameters new_state) {
  double du  = internal_energy_integral(new_state, prev_state.parm, 
      parameters_->cgetT_K());
  double dcv = heat_capac_vol_integral(new_state, prev_state.parm, 
      parameters_->cgetT_K());
  double dif_c = heat_capac_dif_prs_vol(new_state,
      parameters_->cgetT_K(), parameters_->cgetR());
  prev_state.internal_energy += du;
  prev_state.heat_cap_vol    += dcv;
  prev_state.heat_cap_pres   = prev_state.heat_cap_vol + dif_c;
  prev_state.parm = new_state;
  prev_state.Update();
}

// функция вызывается из класса GasParameters_dyn
void Peng_Robinson::update_dyn_params(dyn_parameters &prev_state,
    const parameters new_state, const const_parameters &cp) {
#ifdef GAS_MIX_VARIANT
  set_model_coef(cp);
#endif  // GAS_MIX_VARIANT
  double du  = internal_energy_integral(new_state, prev_state.parm, cp.T_K);
  double dcv = heat_capac_vol_integral(new_state, prev_state.parm, cp.T_K);
  double dif_c = heat_capac_dif_prs_vol(new_state,
      cp.T_K, cp.R);
  prev_state.internal_energy += du;
  prev_state.heat_cap_vol    += dcv;
  prev_state.heat_cap_pres   = prev_state.heat_cap_vol + dif_c;
  prev_state.parm = new_state;
  prev_state.Update();
}

void Peng_Robinson::DynamicflowAccept(DerivateFunctor &df) {
  return df.getFunctor(*this);
}

bool Peng_Robinson::IsValid() const {
  return (parameters_->cgetState() != state_phase::LIQUID);
}

double Peng_Robinson::InitVolume(double p, double t,
    const const_parameters &cp) {
  set_model_coef(cp);
  return get_volume(p, t, cp);
}

void Peng_Robinson::SetVolume(double p, double t) {
  set_parameters(GetVolume(p, t), p, t);
}

void Peng_Robinson::SetPressure(double v, double t) {
  set_parameters(v, GetPressure(v, t), t);
}

#ifndef GAS_MIX_VARIANT
double Peng_Robinson::GetVolume(double p, double t) const {
  if (!is_above0(p, t)) {
    error_ = set_error_code(ERR_CALCULATE_T | ERR_CALC_MODEL_ST);
    return 0.0;
  }
  double alf = std::pow(1.0 + model_coef_k_*(1.0 -
      t / parameters_->cgetT_K()), 2.0);
  std::vector<double> coef {
      1.0,
      model_coef_b_ - parameters_->cgetR()*t/p,
      (model_coef_a_*alf - 2.0 * model_coef_b_ *
          parameters_->cgetR()*t)/p-3.0*model_coef_b_*model_coef_b_,
      std::pow(model_coef_b_, 3.0) + (parameters_->cgetR()*
          t *model_coef_b_*model_coef_b_ - model_coef_a_ * alf *model_coef_b_)/p,
      0.0, 0.0, 0.0};
  CardanoMethod_HASUNIQROOT(&coef[0], &coef[4]);
#ifdef _DEBUG
  if (!is_above0(coef[4])) {
    error_ = set_error_code(ERR_CALCULATE_T | ERR_CALC_MODEL_ST);
    return 0.0;
  }
#endif
  return coef[4];
}

double Peng_Robinson::GetPressure(double v, double t) const {
  if (!is_above0(v, t)) {
    error_ = set_error_code(ERR_CALCULATE_T | ERR_CALC_MODEL_ST);
    return 0.0;
  }
  const double a = std::pow(1.0 + model_coef_k_ * std::pow(1.0 -
      std::sqrt(t / parameters_->cgetT_K()), 2.0), 2.0),
          temp = parameters_->cgetR()*t/(v-model_coef_b_) -
          a * model_coef_a_ /
          (v*v+2.0*model_coef_b_*v -model_coef_b_*model_coef_b_);
  return temp;
}
#else 
double Peng_Robinson::GetVolume(double p, double t) {
  if (!is_above0(p, t)) {
    set_error_code(ERR_CALCULATE_T | ERR_CALC_MODEL_ST);
    return 0.0;
  }
  GasParameters_mix_dyn *gpar = 
      dynamic_cast<GasParameters_mix_dyn *>(parameters_.get());
  if (gpar != nullptr) {
    const parameters_mix &cpm = gpar->GetComponents();
    double volume = 0.0;
    for (auto &x : cpm)
      volume += x.first * get_volume(p, t, x.second.first);
    return volume;
  } else {
    return get_volume(p, t, parameters_->const_params);
  }
  return 0.0;
}

double Peng_Robinson::GetPressure(double v, double t) {
  if (!is_above0(v, t)) {
    set_error_code(ERR_CALCULATE_T | ERR_CALC_MODEL_ST);
    return 0.0;
  }
  GasParameters_mix_dyn *gpar =
      dynamic_cast<GasParameters_mix_dyn *>(parameters_.get());
  if (gpar != nullptr) {
    const parameters_mix &cpm = gpar->GetComponents();
    double pressure = 0.0;
    for (auto &x : cpm)
      pressure += x.first * get_pressure(v, t, x.second.first);
    return pressure;
  } else {
    return get_pressure(v, t, parameters_->const_params);
  }
  return 0.0;
}
#endif  // !GAS_MIX_VARIANT

double Peng_Robinson::GetCoefficient_a() const {
  return model_coef_a_;
}

double Peng_Robinson::GetCoefficient_b() const {
  return model_coef_b_;
}

double Peng_Robinson::GetCoefficient_k() const {
  return model_coef_k_;
}
