#include "model_ideal_gas.h"

#include "common.h"
#include "gasmix_init.h"
#include "gas_description_dynamic.h"
#include "ErrorWrap.h"
#include "models_math.h"

#include <cmath>
#include <memory>

Ideal_Gas::Ideal_Gas(const model_input &mi)
  : modelGeneral(mi.calc_config, mi.gm, mi.bp) {
  set_gasparameters(mi.gpi, this);
  if (!error_.GetErrorCode())
    set_enthalpy();
}

Ideal_Gas *Ideal_Gas::Init(const model_input &mi) {
  if (check_input(mi))
    return nullptr;
  Ideal_Gas *ig = new Ideal_Gas(mi);
  if (ig)
    if (ig->parameters_ == nullptr) {
      delete ig;
      ig = nullptr;
    }
  return ig; 
}

void Ideal_Gas::update_dyn_params(dyn_parameters &prev_state,
    const parameters new_state) {
  prev_state.parm = new_state;
}

void Ideal_Gas::update_dyn_params(dyn_parameters &prev_state,
    const parameters new_state, const const_parameters &cp) {
  assert(0 && "Ideal_Gas update_dyn_params");
}

void Ideal_Gas::DynamicflowAccept(DerivateFunctor &df) {
  df.getFunctor(*this);
}

bool Ideal_Gas::IsValid() const {
  return parameters_->cgetState() == state_phase::GAS;
}

double Ideal_Gas::InitVolume(double p, double t,
    const const_parameters &cp) {
  assert(0);
  return GetVolume(p, t);
}

void Ideal_Gas::SetVolume(double p, double t) {
  set_parameters(GetVolume(p, t), p, t);
}

void Ideal_Gas::SetPressure(double v, double t) {
  set_parameters(v, GetPressure(v, t), t);
}

double Ideal_Gas::GetVolume(double p, double t) {
  if (!is_above0(p, t)) {
    error_.SetError(ERROR_CALC_MODEL_ST);
    return 0.0;
  }
  return  t * parameters_->cgetR() / p;
}

double Ideal_Gas::GetPressure(double v, double t) {
  if (!is_above0(v, t)) {
    error_.SetError(ERROR_CALC_MODEL_ST);
    return 0.0;
  }
  return t * parameters_->cgetR() / v;
}
