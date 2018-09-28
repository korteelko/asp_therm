#include "model_peng_robinson.h"

#include "gas_description_dynamic.h"
#include "models_errors.h"
#include "models_math.h"

#include <iostream>
#include <vector>

/// =========================================================================
/// Peng_robinson methods
/// =========================================================================
void Peng_Robinson::set_model_coef() {
  model_coef_a_ = 0.45724 * std::pow(parameters_->cgetR(), 2.0) *
      std::pow(parameters_->cgetT_K(), 2.0) / parameters_->cgetP_K();
  model_coef_b_ = 0.0778 * parameters_->cgetR() * parameters_->cgetT_K() /
      parameters_->cgetP_K();
  model_coef_k_ = 0.37464 + 1.54226*parameters_->cgetAcentricFactor() -
      0.26992*std::pow(parameters_->cgetAcentricFactor(), 2.0);
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
  // check const_parameters
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
  is_valid &= (!is_above0(prs.pressure, prs.temperature, prs.volume));
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

  // u(p, v, T) = u0 + integrate(....)dv
//   return  u-u0
double Peng_Robinson::internal_energy_integral(const parameters state) {
  double T  = state.temperature,
         Tr = state.temperature / parameters_->const_params.T_K,
         V  = state.volume,
         a  = model_coef_a_,
         b  = model_coef_b_,
         k  = model_coef_k_,
         gm = std::pow(1.0 + k * (1.0 - std::sqrt(Tr)), 2.0),
         sq2 = std::sqrt(2.0);
  double ans = T*T * a * std::sqrt(gm) * k * 
      std::log((V + (1.0 - sq2)*b)/(V + (1.0 + sq2)*b)) +
      a * gm *std::log((V + (1.0 - sq2)*b)/(V + (1.0 + sq2)*b) ) / 
      (2.0 * sq2 * b);
  return ans;
}

// cv(p, v, T) = cv0 + integrate(...)dv
//   return cv - cv0
double Peng_Robinson::heat_capac_vol_integral(const parameters state) {
  double T  = state.temperature,
         Tr = state.temperature / parameters_->const_params.T_K,
         V  = state.volume,
         a  = model_coef_a_,
         b  = model_coef_b_,
         k  = model_coef_k_,
         gm = std::pow(1.0 + k * (1.0 - std::sqrt(Tr)), 2.0);
  double ans = - a * k * std::sqrt(Tr) * (std::sqrt(gm) + k * std::sqrt(gm)) / 
      (2.0 * T*T * (V*V + 2.0*V*b - b*b));
  return ans;
}

// return cp - cv
double Peng_Robinson::heat_capac_dif_prs_vol(const parameters state) {
  double R = parameters_->const_params.R,
         T = state.temperature,
         Tr = state.temperature / parameters_->const_params.T_K,
         V = state.volume,
         a = model_coef_a_,
         b = model_coef_b_,
         k  = model_coef_k_,
         gm = std::pow(1.0 + k * (1.0 - std::sqrt(Tr)), 2.0);
  // сначала числитель
  double num = a * k * std::sqrt(gm * Tr) / (T * (V*V + 2.0 *V*b -b*b)) + R / (V-b); 
  num = std::pow(num, 2.0);
    
  // знаменатель
  double dec = 2.0 * a*(V + b) * gm / std::pow(V*V + 2.0 * b * V - b*b, 2.0) -
      R * T / std::pow(V - b, 2.0); 
  // проверка занменателя на => 0.0
  assert(is_above0(dec) && "Peng_Robinson::heat_capac_dif_prs_vol");
  return num / dec;
}

// функция вызывается из класса GasParameters_dyn
void Peng_Robinson::update_dyn_params(dyn_parameters &prev_state,
    const parameters new_state) {
  // parameters prev_parm = prev_state.parm;
  // internal_energy addition 
  double du  = internal_energy_integral(new_state);
  // heat_capacity_volume addition
  double dcv = heat_capac_vol_integral(new_state);
  // cp - cv
  double dif_c = -new_state.temperature * heat_capac_dif_prs_vol(new_state);
  prev_state.internal_energy += du;
  prev_state.heat_cap_vol    += dcv;
  prev_state.heat_cap_pres   += prev_state.heat_cap_vol + dif_c;
  prev_state.parm = new_state;
  prev_state.Update();
}

void Peng_Robinson::update_dyn_params(dyn_parameters &prev_state,
    const parameters new_state, const const_parameters &cp) {
  assert(0 && "Peng_Robinson update gas_parameters for init GasParameter");
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

double Peng_Robinson::GetCoefficient_a() const {
  return model_coef_a_;
}

double Peng_Robinson::GetCoefficient_b() const {
  return model_coef_b_;
}

double Peng_Robinson::GetCoefficient_k() const {
  return model_coef_k_;
}
