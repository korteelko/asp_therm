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

#include "gas_description.h"
#include "gas_description_dynamic.h"
#include "Logging.h"
#include "models_math.h"

#include <map>
#include <utility>

#ifdef _DEBUG
#  include <iostream>
#endif  // _DEBUG

/** \brief варианты model_info для ГОСТ 30319 и его оригинала - ISO-20765 */
static model_str ng_gost_mi(rg_model_id(rg_model_t::NG_GOST,
    MODEL_SUBTYPE_DEFAULT), 1, 0, "ГОСТ 30319.1-2015");
static model_str ng_gost_iso20765_mi(rg_model_id(rg_model_t::NG_GOST,
    MODEL_GOST_SUBTYPE_ISO_20765), 1, 0, "ГОСТ 30319.1-2015 / ISO-20765");

static model_priority ng_gost_priority(DEF_PRIOR_GOST);
static model_priority ng_gost_iso_priority(DEF_PRIOR_GOST_ISO);

NG_Gost::NG_Gost(const model_input &mi)
  : modelGeneral(mi.ms, mi.gm, mi.bp) {
  set_gasparameters(mi.gpi, this);
  if (mi.mpri.IsSpecified()) {
    priority_ = mi.mpri;
  } else {
    priority_ = (model_config_.model_type.subtype ==
        MODEL_GOST_SUBTYPE_ISO_20765) ? ng_gost_iso_priority : ng_gost_priority;
  }
  ng_pars = dynamic_cast<GasParameters_NG_Gost_dyn *>(parameters_.get());
  if (!ng_pars) {
    error_.SetError(ERROR_INIT_T, "Ошибка инициализации ГОСТ модели");
    error_.LogIt(io_loglvl::err_logs);
  }
  SetVolume(mi.gpi.p, mi.gpi.t);
}

NG_Gost *NG_Gost::Init(const model_input &mi) {
  try {
    check_input(mi);
  } catch (const model_init_exception &e) {
    Logging::Append(e.what());
    return nullptr;
  }
  // only for gas_mix
  if (!(HasGostModelMark(mi.gm)))
    return nullptr;
  return new NG_Gost(mi);
}

model_str NG_Gost::GetModelShortInfo(const rg_model_id &model_type) {
  return (model_type.subtype == MODEL_GOST_SUBTYPE_ISO_20765) ?
      ng_gost_iso20765_mi : ng_gost_mi;

}

model_str NG_Gost::GetModelShortInfo() const {
  return NG_Gost::GetModelShortInfo(model_config_.model_type);
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
  return ng_pars->IsValid();
}

bool NG_Gost::IsValid(parameters prs) const {
  return ng_pars->IsValid(prs);
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
