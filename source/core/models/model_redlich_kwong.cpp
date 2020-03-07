/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#include "model_redlich_kwong.h"

#include "common.h"
#include "gas_description_dynamic.h"
#include "ErrorWrap.h"
#include "models_math.h"

#include <assert.h>

static model_str redlich_kwong_mi(rg_model_t::REDLICH_KWONG2, 0, 1, 0,
    "Модель Редлиха-Квонга");

void Redlich_Kwong2::set_model_coef() {
  model_coef_a_ = 0.42748*std::pow(parameters_->cgetR(), 2.0) *
      std::pow(parameters_->cgetT_K(), 2.5) / parameters_->cgetP_K();
  model_coef_b_ = 0.08664*parameters_->cgetR()*parameters_->cgetT_K() /
      parameters_->cgetP_K();
}

void Redlich_Kwong2::set_model_coef(
    const const_parameters &cp) {
  model_coef_a_ = 0.42748 * std::pow(cp.R, 2.0) *
      std::pow(cp.T_K, 2.5) / cp.P_K;
  model_coef_b_ = 0.08664 * cp.R * cp.T_K / cp.P_K;
}

Redlich_Kwong2::Redlich_Kwong2(const model_input &mi)
  : modelGeneral(mi.calc_config, mi.gm, mi.bp) {
  set_gasparameters(mi.gpi, this);
  if (!error_.GetErrorCode()) {
    set_model_coef();
    if (parameters_->cgetDynSetup() & DYNAMIC_ENTALPHY)
      set_enthalpy();
    SetVolume(mi.gpi.p, mi.gpi.t);
  }
}

Redlich_Kwong2 *Redlich_Kwong2::Init(const model_input &mi) {
  if (check_input(mi))
    return nullptr;
  Redlich_Kwong2 *rk = new Redlich_Kwong2(mi);
  if (rk)
    if (rk->parameters_ == nullptr) {
      delete rk;
      rk = nullptr;
    }
  return rk;
 }

model_str Redlich_Kwong2::GetModelShortInfo() const {
  return redlich_kwong_mi;
}

//  расчёт смотри в ежедневнике
double Redlich_Kwong2::internal_energy_integral(
    const parameters new_state, const parameters old_state) {
  double ans = 3.0 * model_coef_a_ *
      log((new_state.volume * (old_state.volume + model_coef_b_) /
          (old_state.volume * (new_state.volume + model_coef_b_)))) /
      (2.0 * sqrt(new_state.temperature) * model_coef_b_);
  return ans;
}

//   return cv - cv0
double Redlich_Kwong2::heat_capac_vol_integral(
    const parameters new_state, const parameters old_state) {
  double ans = - 3.0 * model_coef_a_ *
      log((new_state.volume * (old_state.volume + model_coef_b_)) /
          (old_state.volume * (new_state.volume + model_coef_b_))) /
      (4.0 * pow(new_state.temperature, 1.5) * model_coef_b_);
  return ans;
}

// return cp - cv
//   исправлено 22_09_2018
double Redlich_Kwong2::heat_capac_dif_prs_vol(
    const parameters new_state, double R) {
  double T = new_state.temperature,
         V = new_state.volume,
         a = model_coef_a_,
         b = model_coef_b_;
  double num = -T * pow(R/(V-b) + a/(2.0*pow(T, 1.5)*(b+V)*V), 2.0);
  double dec = - R*T/pow(V-b, 2.0) + a/(sqrt(T)*V*pow(b+V, 2.0)) + 
      a/(sqrt(T)*V*V*(V+b));
  return num / dec;
}

double Redlich_Kwong2::get_volume(double p, double t,
    const const_parameters &cp) {
  set_model_coef(cp);
  std::vector<double> coef {
      1.0,
      -cp.R*t/p,
      model_coef_a_/(p*std::sqrt(t)) - cp.R *
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

double Redlich_Kwong2::get_pressure(double v, double t,
    const const_parameters &cp) {
  set_model_coef(cp);
  const double temp = cp.R * t / (v - model_coef_b_) -
      model_coef_a_ / (std::sqrt(t)* v *(v + model_coef_b_));
  return temp;
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
  prev_state.parm = new_state;
  prev_state.Update();
}

// visitor
void Redlich_Kwong2::DynamicflowAccept(DerivateFunctor &df) {
  df.getFunctor(*this);
}

bool Redlich_Kwong2::IsValid() const {
  return (parameters_->cgetPressure()/parameters_->cgetP_K() <
      0.5*parameters_->cgetTemperature()/parameters_->cgetT_K());
}

double Redlich_Kwong2::InitVolume(double p, double t,
    const const_parameters &cp) {
  set_model_coef(cp);
  return get_volume(p, t, cp);
}

void Redlich_Kwong2::SetVolume(double p, double t) {
  set_parameters(GetVolume(p, t), p, t);
}

void Redlich_Kwong2::SetPressure(double v, double t) {
  set_parameters(v, GetPressure(v, t), t);
}

double Redlich_Kwong2::GetVolume(double p, double t) {
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

double Redlich_Kwong2::GetPressure(double v, double t) {
  if (!is_above0(v, t)) {
    error_.SetError(ERROR_CALC_MODEL_ST);
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
