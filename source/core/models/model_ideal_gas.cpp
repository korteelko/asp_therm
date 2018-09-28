#include "model_ideal_gas.h"

#include "gas_description_dynamic.h"
#include "models_errors.h"
#include "models_math.h"

#include <cmath>

IdealGas::IdealGas(modelName mn, parameters prs, const_parameters cgp,
    dyn_parameters dgp, binodalpoints bp)
  : modelGeneral::modelGeneral(mn, prs, cgp, dgp, bp) {
  parameters_ = std::unique_ptr<GasParameters>(
      GasParameters_dyn::Init(prs, cgp, dgp, this));
}

IdealGas::IdealGas(modelName mn, parameters prs, parameters_mix components,
    binodalpoints bp)
  : modelGeneral::modelGeneral(mn, prs, components, bp) {
  parameters_ = std::unique_ptr<GasParameters>(
      GasParameters_mix_dyn::Init(prs, components, this));
}

IdealGas *IdealGas::Init(modelName mn, parameters prs,
    const_parameters cgp,
    dyn_parameters dgp, binodalpoints bp) {
  // check const_parameters
  if (!is_above0(cgp.acentricfactor, cgp.molecularmass,
      cgp.P_K, cgp.R, cgp.T_K,cgp.V_K)) {
    set_error_code(ERR_INIT_T | ERR_INIT_ZERO_ST);
    return nullptr;
  }
  IdealGas *tmp = new IdealGas(mn, prs, cgp, dgp, bp);
  if (tmp == nullptr)
    return nullptr;
  if (tmp->getGasParameters() == nullptr) {
    delete tmp;
    return nullptr;
  }
  return tmp;
}

IdealGas *IdealGas::Init(modelName mn, parameters prs,
    parameters_mix components,
    binodalpoints bp) {
  // check const_parameters
  if (components.empty())
    return nullptr;
  IdealGas *tmp = new IdealGas(mn, prs, components, bp);
  if (tmp == nullptr)
    return nullptr;
  if (tmp->getGasParameters() == nullptr) {
    delete tmp;
    return nullptr;
  }
  return tmp;
}

// Для идеального газа теплоёмкость газа постоянна
// For ideal gas heat capacity is const
void IdealGas::update_dyn_params(dyn_parameters &prev_state,
    const parameters new_state) {
  prev_state.parm = new_state;
}
void IdealGas::update_dyn_params(dyn_parameters &prev_state,
    const parameters new_state, const const_parameters &cp) {
  assert(0 && "IdealGas update_dyn_params");
}

bool IdealGas::IsValid() const {
  return parameters_->cgetState() == state_phase::GAS;
}

// visitor
void IdealGas::DynamicflowAccept(DerivateFunctor &df) {
  df.getFunctor(*this);
}

void IdealGas::SetVolume(double p, double t) {
  setParameters(GetVolume(p, t), p, t);
}

void IdealGas::SetPressure(double v, double t) {
  setParameters(v, GetPressure(v, t), t);
}

/*
void idealGas::setTemperature(double v, double p) {
  if (!is_above0(p, v)) {
    set_error_code(ERR_CALCULATE);
    return;
  }
  double temp = p * v / parameters_->cgetR();
  setParameters(v, p, temp);
}
*/

double IdealGas::GetVolume(double p, double t) const {
  if (!is_above0(p, t)) {
    set_error_code(ERR_CALCULATE_T | ERR_CALC_MODEL_ST);
    return 0.0;
  }
  return  t * parameters_->cgetR() / p;
}

double IdealGas::GetPressure(double v, double t) const {
  if (!is_above0(v, t)) {
    set_error_code(ERR_CALCULATE_T | ERR_CALC_MODEL_ST);
    return 0.0;
  }
  return t * parameters_->cgetR() / v;
}
