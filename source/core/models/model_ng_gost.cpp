#include "model_ng_gost.h"

#include "gas_description.h"
#include "gas_description_dynamic.h"
#include "models_errors.h"
#include "models_math.h"

#include <map>
#include <utility>

#ifdef _DEBUG
#  include <iostream>
#endif  // _DEBUG

NG_Gost::NG_Gost(const model_input &mi) 
  : modelGeneral(mi.gm, mi.bp) {
  if (!set_gasparameters(mi.gpi, this))
    return;
}

NG_Gost *NG_Gost::Init(const model_input &mi) {
  reset_error();
  if (!check_input(mi))
    return nullptr;
  // only for gas_mix
  if (!(mi.gm & GAS_NG_GOST_MARK))
    return nullptr;
  return new NG_Gost(mi);
}

void NG_Gost::DynamicflowAccept(class DerivateFunctor &df) {}

void NG_Gost::update_dyn_params(dyn_parameters &prev_state,
    const parameters new_state) {}

void NG_Gost::update_dyn_params(dyn_parameters &prev_state,
    const parameters new_state, const const_parameters &cp) {}

bool NG_Gost::IsValid() const {
  assert(0);
  return false;
}

double NG_Gost::InitVolume(double p, double t,
    const const_parameters &cp) {
  assert(0);
  return 0.0;
}

void NG_Gost::SetVolume(double p, double t) {
  parameters_->csetParameters(0.0, p, t, state_phase::GAS);
}

void NG_Gost::SetPressure(double v, double t) {
  set_error_message("invalid operation for this model");
}

#ifndef GAS_MIX_VARIANT
double NG_Gost::GetVolume(double p, double t) const {
#else
double NG_Gost::GetVolume(double p, double t) {
#endif  // !GAS_MIX_VARIANT
  parameters_->csetParameters(0.0, p, t, state_phase::GAS);
  return parameters_->cgetVolume();
}

#ifndef GAS_MIX_VARIANT
double NG_Gost::GetPressure(double v, double t) const {
#else
double NG_Gost::GetPressure(double v, double t) {
#endif  // !GAS_MIX_VARIANT
  set_error_message("invalid operation for gost model");
  return 0.0;
}

/*
void DynamicflowAccept(class DerivateFunctor &df);
bool IsValid() const override;
double InitVolume(double p, double t,
    const const_parameters &cp) override;
void SetVolume(double p, double t)      override;
void SetPressure(double v, double t)    override;
#ifndef GAS_MIX_VARIANT
double GetVolume(double p, double t)    const override;
double GetPressure(double v, double t)  const override;
#else
double GetVolume(double p, double t)    override;
double GetPressure(double v, double t)  override;
#endif  // !GAS_MIX_VARIANT
*/