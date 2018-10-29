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

namespace {
// typedef std::pair<double, double> mix_valid_limits_t;
  struct max_valid_limits_t {
    double min,
           max;
  };
  std::map<gas_t, max_valid_limits_t> mix_valid_molar =
      std::map<gas_t, max_valid_limits_t> {
    {GAS_TYPE_METHANE, {0.7, 0.99999}},
    {GAS_TYPE_ETHANE,  {0.0, 0.1}},
    {GAS_TYPE_PROPANE, {0.0, 0.035}},
    {GAS_TYPE_BUTAN,   {0.0, 0.015}},
    {GAS_TYPE_PENTANE, {0.0, 0.005}},
    {GAS_TYPE_HEXANE,  {0.0, 0.001}},
    {GAS_TYPE_NITROGEN, {0.0, 0.2}},
    {GAS_TYPE_CARBON_DIOXIDE, {0.0, 0.2}},
    {GAS_TYPE_HELIUM, {0.0, 0.005}},
    {GAS_TYPE_HYDROGEN, {0.0, 0.1}},
    // SUM of others
    {GAS_TYPE_UNDEFINED, {0.0, 0.0015}}
  };
}  // anonymus namespace

NG_Gost::NG_Gost(const model_input &mi) 
  : modelGeneral(mi.gm, mi.bp), model_coef_z_(0.0) {
  set_model_coef();
}

NG_Gost *NG_Gost::Init(const model_input &mi) {
  reset_error();
  if (!check_input(mi))
    return nullptr;
  // only for gas_mix
  if (!(mi.gm & GAS_MIX_MARK))
    return nullptr;
}

void NG_Gost::set_model_coef() { 
  model_coef_z_ = 1.0;
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