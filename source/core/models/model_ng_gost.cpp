#include "model_ng_gost.h"

#include "common.h"
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
  assert(0);
  // init GasParameters_NG_Gost_dyn
}

NG_Gost *NG_Gost::Init(const model_input &mi) {
  reset_error();
  if (!check_input(mi))
    return nullptr;
  // only for gas_mix
  if (!(mi.gm & GAS_MIX_MARK))
    return nullptr;
  return new NG_Gost(mi);
}

void NG_Gost::set_model_coef() { 
  assert(0);
}

void NG_Gost::set_model_coef(const const_parameters &cp) { assert(0);}

void NG_Gost::update_dyn_params(dyn_parameters &prev_state,
    const parameters new_state) { assert(0);}

void NG_Gost::update_dyn_params(dyn_parameters &prev_state,
    const parameters new_state, const const_parameters &cp) { assert(0);}

double NG_Gost::internal_energy_integral(const parameters new_state,
    const parameters prev_state) { assert(0);}

double NG_Gost::heat_capac_vol_integral(const parameters new_state,
    const parameters prev_state) { assert(0);}

double NG_Gost::heat_capac_dif_prs_vol(const parameters new_state, double R) {
  assert(0);
}

double NG_Gost::get_volume(double p, double t, const const_parameters &cp) {
  assert(0);
}

double NG_Gost::get_pressure(double v, double t, const const_parameters &cp) {
  assert(0);
}

void NG_Gost::DynamicflowAccept(class DerivateFunctor &df) {
  assert(0);
}

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
  assert(0);
}

void NG_Gost::SetPressure(double v, double t) {
  set_error_message("invalid operation for this model");
}

#ifndef GAS_MIX_VARIANT
  double NG_Gost::GetVolume(double p, double t)    const override;
  double NG_Gost::GetPressure(double v, double t)  const override;
#else
  double NG_Gost::GetVolume(double p, double t) {
    assert(0);
    return 0.0;
  }

  double NG_Gost::GetPressure(double v, double t) {
    set_error_message("invalid operation for gost model");
    return 0.0;
  }
#endif  // !GAS_MIX_VARIANT

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