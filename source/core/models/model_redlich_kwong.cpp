#include "model_redlich_kwong.h"

#include "gas_description_dynamic.h"
#include "models_math.h"
#include "models_errors.h"
#include "models_math.h"

#include <cmath>
#ifdef _DEBUG
#  include <iostream>
#endif  // _DEBUG

#include <assert.h>

void Redlich_Kwong2::set_model_coef() {
  model_coef_a_ = 0.42748*std::pow(parameters_->cgetR(), 2.0) *
      std::pow(parameters_->cgetT_K(), 2.5) / parameters_->cgetP_K();
  model_coef_b_ = 0.08664*parameters_->cgetR()*parameters_->cgetT_K() /
      parameters_->cgetP_K();
}

void Redlich_Kwong2::set_model_coef(
    const const_parameters &cp) {
  model_coef_a_ = 0.42748 * std::pow(cp.R, 2.0) *
      std::pow(cp.T_K, 2.5) / cp.P_K,
  model_coef_b_ = 0.08664 * cp.R * cp.T_K / cp.P_K;
}

Redlich_Kwong2::Redlich_Kwong2(modelName mn, parameters prs,
    const_parameters cgp, dyn_parameters dgp, binodalpoints bp)
  : modelGeneral::modelGeneral(mn, prs, cgp, dgp, bp) {
  parameters_ = std::unique_ptr<GasParameters>(
      GasParameters_dyn::Init(prs, cgp, dgp, this));
  set_model_coef();
}

Redlich_Kwong2::Redlich_Kwong2(modelName mn, parameters prs,
    parameters_mix components, binodalpoints bp)
  : modelGeneral(mn, prs, components, bp) {
  parameters_ = std::unique_ptr<GasParameters>(
      GasParameters_mix_dyn::Init(prs, components, this));
  set_model_coef();
}

Redlich_Kwong2 *Redlich_Kwong2::Init(modelName mn, parameters prs,
    const_parameters cgp, dyn_parameters dgp, binodalpoints bp) {
  reset_error();
  bool is_valid = is_valid_cgp(cgp) && is_valid_dgp(dgp);
  is_valid &= is_above0(prs.pressure, prs.temperature, prs.volume);
  if (!is_valid) {
    set_error_code(ERR_INIT_T | ERR_INIT_ZERO_ST);
    return nullptr;
  }
  Redlich_Kwong2 *rk = new Redlich_Kwong2(mn, prs, cgp, dgp, bp);
  // окончательная проверка
  if (rk)
    if (rk->parameters_ == nullptr) {
      set_error_code(ERR_INIT_T);
      delete rk;
      rk = nullptr;
    }
  return rk;
}

Redlich_Kwong2 *Redlich_Kwong2::Init(modelName mn, parameters prs,
    parameters_mix components, binodalpoints bp) {
  // check const_parameters
  reset_error();
  bool is_valid = !components.empty();
  if (is_valid)
    is_valid = is_above0(prs.pressure, prs.temperature, prs.volume);
  if (!is_valid) {
    set_error_code(ERR_INIT_T | ERR_INIT_ZERO_ST | ERR_GAS_MIX);
    return nullptr;
  }
  Redlich_Kwong2 *rk = new Redlich_Kwong2(mn, prs, components, bp);
  if (rk)
    if (rk->parameters_ == nullptr) {
      set_error_code(ERR_INIT_T);
      delete rk;
      rk = nullptr;
    }
  return rk;
 }

//  расчёт смотри в ежедневнике
//  UPD: Matlab/GNUOctave files in dir somemath

  // u(p, v, T) = u0 + integrate(....)dv
//   return  u-u0
double Redlich_Kwong2::internal_energy_integral(const parameters new_state,
    const parameters old_state) {
  double ans = 3.0 * model_coef_a_ *
      log((new_state.volume * (old_state.volume + model_coef_b_) /
          (old_state.volume * (new_state.volume + model_coef_b_)))) /
      (2.0 * sqrt(new_state.temperature) * model_coef_b_);
  return ans;
}

// cv(p, v, T) = cv0 + integrate(...)dv
//   return cv - cv0
double Redlich_Kwong2::heat_capac_vol_integral(const parameters new_state,
    const parameters old_state) {
  double ans = - 3.0 * model_coef_a_ *
      log((new_state.volume * (old_state.volume + model_coef_b_)) /
          (old_state.volume * (new_state.volume + model_coef_b_))) /
      (4.0 * pow(new_state.temperature, 1.5) * model_coef_b_);
  return ans;
}

// return cp - cv
//   исправлено 22_09_2018
double Redlich_Kwong2::heat_capac_dif_prs_vol(const parameters new_state,
    double R) {
  double T = new_state.temperature,
         V = new_state.volume,
         a = model_coef_a_,
         b = model_coef_b_;
  // сначала числитель
  // double num = 4.0 * R*R * T*T*T * V*V* (V + b)*(V + b) +
  //    4.0*(V*V -b*b)*V*R*a*pow(T, 1.5)  +  a*a * (V-b)*(V-b);
  double num = -T * pow(R/(V-b) + a/(2.0*pow(T, 1.5)*(b+V)*V), 2.0);
  // знаменатель
  // double dec = 4.0 * a * (2.0*V*V*V - 3*b*V*V + b*b*b)*pow(T, 1.5) - 
  //    4.0 * V*V * R * T*T*T*T * (V+b)*(V+b);
  double dec = - R*T/pow(V-b, 2.0) + a/(sqrt(T)*V*pow(b+V, 2.0)) + 
      a/(sqrt(T)*V*V*(V+b));
  return num / dec;
}

void Redlich_Kwong2::update_dyn_params(dyn_parameters &prev_state,
    const parameters new_state) {
  // parameters prev_parm = prev_state.parm;
  // internal_energy addition 
  double du  = internal_energy_integral(new_state, prev_state.parm);
  // heat_capacity_volume addition
  double dcv = heat_capac_vol_integral(new_state, prev_state.parm);
  // cp - cv
  double dif_c = heat_capac_dif_prs_vol(new_state, 
      parameters_->const_params.R);
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
void Redlich_Kwong2::update_dyn_params(dyn_parameters &prev_state,
    const parameters new_state, const const_parameters &cp) {
  set_model_coef(cp);
  double du  = internal_energy_integral(new_state, prev_state.parm);
  // heat_capacity_volume addition
  double dcv = heat_capac_vol_integral(new_state, prev_state.parm);
  // cp - cv
  double dif_c = heat_capac_dif_prs_vol(new_state, cp.R);
  prev_state.internal_energy += du;
  prev_state.heat_cap_vol    += dcv;
  prev_state.heat_cap_pres   = prev_state.heat_cap_vol + dif_c;
#ifdef _DEBUG
  std::cerr << "\nUPDATE DYN_PARAMETERS: dcv " << dcv << " dif_c " 
      << dif_c << std::endl; 
#endif  // _DEBUG
  prev_state.parm = new_state;
  prev_state.Update();
}
  // 20_09_2018
  // return num / dec;

// visitor
void Redlich_Kwong2::DynamicflowAccept(DerivateFunctor &df) {
  df.getFunctor(*this);
}

bool Redlich_Kwong2::IsValid() const {
  return (parameters_->cgetPressure()/parameters_->cgetP_K() <
      0.5*parameters_->cgetTemperature()/parameters_->cgetT_K());
}

void Redlich_Kwong2::SetVolume(double p, double t) {
  setParameters(GetVolume(p, t), p, t);
}

void Redlich_Kwong2::SetPressure(double v, double t) {
  setParameters(v, GetPressure(v, t), t);
}

double Redlich_Kwong2::GetVolume(double p, double t) const {
  if (!is_above0(p, t)) {
    set_error_code(ERR_CALCULATE_T | ERR_CALC_MODEL_ST);
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
  CardanoMethod_HASUNIQROOT(&coef[0], &coef[4]);
#ifdef _DEBUG
  if (!is_above0(coef[4])) {
    set_error_code(ERR_CALCULATE_T | ERR_CALC_MODEL_ST);
    return 0.0;
  }
#endif
  return coef[4];
}

double Redlich_Kwong2::GetPressure(double v, double t) const {
  if (!is_above0(v, t)) {
    set_error_code(ERR_CALCULATE_T | ERR_CALC_MODEL_ST);
    return 0.0;
  }
  const double temp = parameters_->cgetR() * t / (v - model_coef_b_) -
      model_coef_a_ / (std::sqrt(t)* v *(v + model_coef_b_));
  return temp;
}

double Redlich_Kwong2::GetCoefficient_a() const {
  return model_coef_a_;
}

double Redlich_Kwong2::GetCoefficient_b() const {
  return model_coef_b_;
}
