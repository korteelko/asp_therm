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

Peng_Robinson::Peng_Robinson(modelName mn, parameters prs,
    const_parameters cgp, dyn_parameters dgp, binodalpoints bp)
  : modelGeneral::modelGeneral(mn, prs, cgp, dgp, bp) {
  parameters_ = std::unique_ptr<GasParameters>(
      GasParameters_dyn::Init(prs, cgp, dgp, this));
  set_model_coef();
}

Peng_Robinson::Peng_Robinson(modelName mn, parameters prs,
    parameters_mix components, binodalpoints bp) 
    : modelGeneral::modelGeneral(mn, prs, components, bp) {
  parameters_ = std::unique_ptr<GasParameters>(
      GasParameters_mix_dyn::Init(prs, components, this));
  set_model_coef();
}

Peng_Robinson *Peng_Robinson::Init(modelName mn, parameters prs,
    const_parameters cgp, dyn_parameters dgp, binodalpoints bp) {
  reset_error();
  bool is_valid = is_valid_cgp(cgp) && is_valid_dgp(dgp);
  is_valid &= (is_above0(prs.pressure, prs.temperature, prs.volume));
  if (!is_valid) {
    set_error_code(ERR_INIT_T | ERR_INIT_ZERO_ST);
    return nullptr;
  }
  Peng_Robinson *pr = new Peng_Robinson(mn, prs, cgp, dgp, bp);
  if (pr)
    if (pr->parameters_ == nullptr) {
      set_error_code(ERR_INIT_T);
      delete pr;
      pr = nullptr;
    }
  return pr; 
}

Peng_Robinson *Peng_Robinson::Init(modelName mn, parameters prs,
    parameters_mix components, binodalpoints bp) {
  reset_error();
  // для проверки установленных доль
  bool is_valid = !components.empty();
  if (is_valid)
    is_valid = is_above0(prs.pressure, prs.temperature, prs.volume);
  if (!is_valid) {
    set_error_code(ERR_INIT_T | ERR_INIT_ZERO_ST | ERR_GAS_MIX);
    return nullptr;
  }
  Peng_Robinson *pr = new Peng_Robinson(mn, prs, components, bp);
  if (pr)
    if (pr->parameters_ == nullptr) {
      set_error_code(ERR_INIT_T);
      delete pr;
      pr = nullptr;
    }
  return pr; 
}

//  расчёт смотри в ежедневнике
//  UPD: Matlab/GNUOctave files in dir somemath
/* double log_pr(double v, double cb) {
  return log(v + cb);
} */

double Peng_Robinson::log_pr(double v, bool is_posit) {
  return (is_posit) ? (1.0 + sq2) * model_coef_b_ + v : 
      (1.0 - sq2) * model_coef_b_ + v;
}

  // u(p, v, T) = u0 + integrate(....)dv
//   return  u-u0
double Peng_Robinson::internal_energy_integral(const parameters new_state,
    const parameters old_state, const double Tk) {
  double t  = new_state.temperature,
         Tr = new_state.temperature / Tk,
         vf = new_state.volume,
         v0 = old_state.volume,
         a  = model_coef_a_,
         b  = model_coef_b_,
         k  = model_coef_k_;
  /* 26_09_2018
         gm = std::pow(1.0 + k * (1.0 - std::sqrt(Tr)), 2.0);
  double ans = T*T * a * std::sqrt(gm) * k * 
      std::log((V + (1.0 - sq2)*b)/(V + (1.0 + sq2)*b)) +
      a * gm *std::log((V + (1.0 - sq2)*b)/(V + (1.0 + sq2)*b) ) / 
      (2.0 * sq2 * b);
  */
  double ans = sq2 * 0.25 * a * k * t * t * 
      (-k*sqrt(Tr) + k - sqrt(Tr) + 2.0 + 1.0/k) * 
      log(log_pr(vf, false) * log_pr(v0, true) / 
          log_pr(vf, true) / log_pr(v0, false)) /
      (b * t * t);
  return ans;
}

// cv(p, v, T) = cv0 + integrate(...)dv
//   return cv - cv0
double Peng_Robinson::heat_capac_vol_integral(const parameters new_state,
    const parameters prev_state, const double Tk) {
  double t  = new_state.temperature,
         Tr = new_state.temperature / Tk,
         vf = new_state.volume,
         v0 = prev_state.volume,
         a  = model_coef_a_,
         b  = model_coef_b_,
         k  = model_coef_k_;
  /* 26_09_2018
         gm = std::pow(1.0 + k * (1.0 - std::sqrt(Tr)), 2.0);
  double ans = - a * k * std::sqrt(Tr) * (std::sqrt(gm) + k * std::sqrt(gm)) / 
      (2.0 * T*T * (V*V + 2.0*V*b - b*b));
  */
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
  /* 30_09_2018 */
  /*
  double gm = k*(-sqrt(Tr) + 1.0) + 1.0;
  // сначала числитель
  double num = -t * pow(R/(v-b) + a*k*sqrt(Tr)*gm / 
      (t * (-b*b + 2.0*b*v *v*v)), 2.0);
  // знаменатель
  double dec = -R * t / pow(v-b, 2.0) -
      2.0*a*(-b-v)*gm*gm / pow(-b*b+2*b*v+v*v, 2.0);
  return num / dec;
  */
  return  t * pow((R/(-b + v) + a*k*sqrt(Tr)*(k*(-sqrt(Tr) + 1.0) + 1.0) /
      (t*(-b*b + 2.0*b*v + v*v))), 2.0) / 
      (R*t / pow((-b + v), 2.0) - a*(-2.0*b - 2.0*v)*
      pow((k*(-sqrt(Tr) + 1.0) + 1.0), 2.0) / pow((-b*b + 2.0*b*v + v*v), 2.0));
}


double Peng_Robinson::get_volume(double p, double t, const_parameters &cp) {
  double alf = std::pow(1.0 + model_coef_k_*(1.0 -
      t / cp.T_K), 2.0);
  std::vector<double> coef {
      1.0,
      model_coef_b_ - cp.R*t/p,
      (model_coef_a_*alf - 2.0f * model_coef_b_ *
          cp.R*t)/p - 3.0f*model_coef_b_*model_coef_b_,
      std::pow(model_coef_b_, 3.0f) + (cp.R*
          t*model_coef_b_*model_coef_b_ - model_coef_a_ * alf *model_coef_b_)/p,
      0.0, 0.0, 0.0};
  CardanoMethod_HASUNIQROOT(&coef[0], &coef[4]);
#ifdef _DEBUG
  if (!is_above0(coef[4])) {
    set_error_code(ERR_CALCULATE_T | ERR_CALC_MODEL_ST);
    return 0.0;
  }
#endif
  return coef[4];
}

void Peng_Robinson::update_dyn_params(dyn_parameters &prev_state,
    const parameters new_state) {
  // parameters prev_parm = prev_state.parm;
  // internal_energy addition 
  double du  = internal_energy_integral(new_state, prev_state.parm, 
      parameters_->cgetT_K());
  // heat_capacity_volume addition
  double dcv = heat_capac_vol_integral(new_state, prev_state.parm, 
      parameters_->cgetT_K());
  // cp - cv
  double dif_c = heat_capac_dif_prs_vol(new_state,
      parameters_->cgetT_K(), parameters_->cgetR());
  prev_state.internal_energy += du;
  prev_state.heat_cap_vol    += dcv;
  prev_state.heat_cap_pres   = prev_state.heat_cap_vol + dif_c;
#ifdef _DEBUG
  std::cerr << "\nUPDATE DYN_PARAMETERS2: dcv " << dcv << " dif_c "
      << dif_c << std::endl; 
#endif  // _DEBUG
  prev_state.parm = new_state;
  prev_state.Update();
}

// функция вызывается из класса GasParameters_dyn
void Peng_Robinson::update_dyn_params(dyn_parameters &prev_state,
    const parameters new_state, const const_parameters &cp) {
  set_model_coef(cp);
  double du  = internal_energy_integral(new_state, prev_state.parm, cp.T_K);
  // heat_capacity_volume addition
  double dcv = heat_capac_vol_integral(new_state, prev_state.parm, cp.T_K);
  // cp - cv
  double dif_c = heat_capac_dif_prs_vol(new_state,
      cp.T_K, cp.R);
  prev_state.internal_energy += du;
  prev_state.heat_cap_vol    += dcv;
  prev_state.heat_cap_pres   = prev_state.heat_cap_vol + dif_c;
#ifdef _DEBUG
  std::cerr << "\nUPDATE DYN_PARAMETERS2: dcv " << dcv << " dif_c "
      << dif_c << std::endl; 
#endif  // _DEBUG
  prev_state.parm = new_state;
  prev_state.Update();
}

void Peng_Robinson::DynamicflowAccept(DerivateFunctor &df) {
  return df.getFunctor(*this);
}

bool Peng_Robinson::IsValid() const {
  return (parameters_->cgetState() != state_phase::LIQUID);
}

void Peng_Robinson::SetVolume(double p, double t) {
  setParameters(GetVolume(p, t), p, t);
}

void Peng_Robinson::SetPressure(double v, double t) {
  setParameters(v, GetPressure(v, t), t);
}

#ifndef GAS_MIX_VARIANT
double Peng_Robinson::GetVolume(double p, double t) const {
  if (!is_above0(p, t)) {
    set_error_code(ERR_CALCULATE_T | ERR_CALC_MODEL_ST);
    return 0.0;
  }
  double alf = std::pow(1.0 + model_coef_k_*(1.0 -
      t / parameters_->cgetT_K()), 2.0);
  std::vector<double> coef {
      1.0,
      model_coef_b_ - parameters_->cgetR()*t/p,
      (model_coef_a_*alf - 2.0f * model_coef_b_ *
          parameters_->cgetR()*t)/p-3.0f*model_coef_b_*model_coef_b_,
      std::pow(model_coef_b_, 3.0f) + (parameters_->cgetR()*
          t *model_coef_b_*model_coef_b_ - model_coef_a_ * alf *model_coef_b_)/p,
      0.0, 0.0, 0.0};
  CardanoMethod_HASUNIQROOT(&coef[0], &coef[4]);
#ifdef _DEBUG
  if (!is_above0(coef[4])) {
    set_error_code(ERR_CALCULATE_T | ERR_CALC_MODEL_ST);
    return 0.0;
  }
#endif
  return coef[4];
}

double Peng_Robinson::GetPressure(double v, double t) const {
  if (!is_above0(v, t)) {
    set_error_code(ERR_CALCULATE_T | ERR_CALC_MODEL_ST);
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
  GasParameters_mix_dyn *gpar = dynamic_cast<GasParameters_mix_dyn *>(parameters_.get());
  if (gpar != nullptr) {
    const parameters_mix &cpm = gpar->GetComponents();

    set_model_coef(cp);
    assert(0);
  } else {
    return get_volume(p, t, parameters_->const_params);
  }
  return 0.0;
}

double Peng_Robinson::GetPressure(double v, double t) {
  assert(0);
  if (!is_above0(v, t)) {
    set_error_code(ERR_CALCULATE_T | ERR_CALC_MODEL_ST);
    return 0.0;
  }
  const double a = std::pow(1.0 + model_coef_k_ * std::pow(1.0 -
      std::sqrt(t / parameters_->cgetT_K()), 2.0), 2.0),
          temp = parameters_->cgetR()*t/(v-model_coef_b_) -
          a * model_coef_a_ /
          (v*v+2.0*model_coef_b_*v -model_coef_b_*model_coef_b_);
  return temp;
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
