/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#include "model_ng_gost.h"

#include "common.h"
#include "gas_description.h"
#include "gas_description_dynamic.h"
#include "gasmix_init.h"
#include "ErrorWrap.h"
#include "models_math.h"

#include <map>
#include <utility>

#ifdef _DEBUG
#  include <iostream>
#endif  // _DEBUG

/** \brief варианты model_info для ГОСТ 30319 и его оригинала - ISO-20765 */
static model_str ng_gost_mi(rg_model_t::NG_GOST, MODEL_SUBTYPE_DEFAULT,
    1, 0, "ГОСТ 30319.1-2015");
static model_str ng_gost_iso20765_mi(rg_model_t::NG_GOST,
    MODEL_SUBTYPE_ISO_20765, 1, 0, "ГОСТ 30319.1-2015 / ISO-20765");

NG_Gost::NG_Gost(const model_input &mi)
  : modelGeneral(mi.calc_config, mi.gm, mi.bp) {
  set_gasparameters(mi.gpi, this);
}

NG_Gost *NG_Gost::Init(const model_input &mi) {
  if (check_input(mi))
    return nullptr;
  // only for gas_mix
  if (!(HasGostModelMark(mi.gm)))
    return nullptr;
  return new NG_Gost(mi);
}

model_str NG_Gost::GetModelShortInfo() const {
  return (calc_config_.EnableISO20765()) ? ng_gost_iso20765_mi : ng_gost_mi;
}

void NG_Gost::DynamicflowAccept(class DerivateFunctor &df) {
  return df.getFunctor(*this);
}

void NG_Gost::update_dyn_params(dyn_parameters &prev_state,
    const parameters new_state) {
  (void)prev_state; (void)new_state;
}

void NG_Gost::update_dyn_params(dyn_parameters &prev_state,
    const parameters new_state, const const_parameters &cp) {
  (void)prev_state; (void)new_state; (void)cp;
}

bool NG_Gost::IsValid() const {
#if defined(_DEBUG)
  return true;
#else
  assert(0);
  return false;
#endif  // _debug
}

double NG_Gost::InitVolume(double p, double t,
    const const_parameters &cp) {
  (void)p; (void)t; (void)cp;
  assert(0);
  return 0.0;
}

void NG_Gost::SetVolume(double p, double t) {
  parameters_->csetParameters(0.0, p, t, state_phase::GAS);
}

void NG_Gost::SetPressure(double v, double t) {
  (void)v; (void)t;
  error_.SetError(ERROR_CALC_MODEL_ST, "invalid operation for this model");
}

double NG_Gost::GetVolume(double p, double t) {
  parameters_->csetParameters(0.0, p, t, state_phase::GAS);
  return parameters_->cgetVolume();
}

double NG_Gost::GetPressure(double v, double t) {
  (void) v; (void) t;
  error_.SetError(ERROR_CALC_MODEL_ST, "invalid operation for gost model");
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
