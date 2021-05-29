/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020-2021 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#include "model_ng_gost.h"

#include "asp_utils/Logging.h"
#include "gas_description.h"
#include "gas_description_dynamic.h"
#include "models_math.h"

#include <map>
#include <utility>

#ifdef _DEBUG
#include <iostream>
#endif  // _DEBUG

/** \brief варианты model_info для ГОСТ 30319 и его оригинала - ISO-20765 */
static model_str ng_gost_mi(rg_model_id(rg_model_t::NG_GOST,
                                        MODEL_SUBTYPE_DEFAULT),
                            1,
                            0,
                            "ГОСТ 30319.1-2015");
static model_str ng_gost_iso20765_mi(rg_model_id(rg_model_t::NG_GOST,
                                                 MODEL_GOST_SUBTYPE_ISO_20765),
                                     1,
                                     0,
                                     "ГОСТ 30319.1-2015 / ISO-20765");

static model_priority ng_gost_priority(DEF_PRIOR_GOST);
static model_priority ng_gost_iso_priority(DEF_PRIOR_GOST_ISO);

NG_Gost::NG_Gost(const model_input& mi) : modelGeneral(mi.ms, mi.gm, mi.bp) {
  set_gasparameters(mi.gpi, this);
  if (mi.mpri.IsSpecified()) {
    priority_ = mi.mpri;
  } else {
    priority_ =
        (model_config_.model_type.subtype == MODEL_GOST_SUBTYPE_ISO_20765)
            ? ng_gost_iso_priority
            : ng_gost_priority;
  }
  SetVolume(mi.gpi.p, mi.gpi.t);
}

NG_Gost* NG_Gost::Init(const model_input& mi) {
  NG_Gost* res = nullptr;
  bool is_valid = HasGostModelMark(mi.gm);
  try {
    check_input(mi);
  } catch (const model_init_exception& e) {
    Logging::Append(e.what());
    is_valid = false;
  }
  if (is_valid) {
    res = new NG_Gost(mi);
    if (res) {
      if (res->error_.GetErrorCode()) {
        delete res;
        res = nullptr;
      }
    }
  }
  return res;
}

model_str NG_Gost::GetModelShortInfo(const rg_model_id& model_type) {
  return (model_type.subtype == MODEL_GOST_SUBTYPE_ISO_20765)
             ? ng_gost_iso20765_mi
             : ng_gost_mi;
}

model_str NG_Gost::GetModelShortInfo() const {
  return NG_Gost::GetModelShortInfo(model_config_.model_type);
}

void NG_Gost::DynamicflowAccept(class DerivateFunctor& df) {
  return df.getFunctor(*this);
}

void NG_Gost::update_dyn_params(dyn_parameters& prev_state,
                                const parameters new_state) {
  (void)prev_state;
  (void)new_state;
}

void NG_Gost::update_dyn_params(dyn_parameters& prev_state,
                                const parameters new_state,
                                const const_parameters& cp) {
  (void)prev_state;
  (void)new_state;
  (void)cp;
}

bool NG_Gost::IsValid() const {
  return IsValid(parameters_->cgetParameters());
}

bool NG_Gost::IsValid(parameters prs) const {
  auto* p = dynamic_cast<GasParametersGost30319Dyn*>(parameters_.get());
  if (p) {
    return p->IsValid(prs);
  } else {
    Logging::Append(io_loglvl::debug_logs,
                    "Ошибка приведения типов в ГОСТ модели");
  }
  return false;
}

void NG_Gost::SetVolume(double p, double t) {
  if (is_status_aval(status_))
    parameters_->csetParameters(0.0, p, t, state_phase::GAS);
}

void NG_Gost::SetPressure(double v, double t) {
  (void)v;
  (void)t;
  Logging::Append(ERROR_CALC_MODEL_ST, "invalid operation for this model");
}

double NG_Gost::GetVolume(double p, double t) {
  if (is_status_aval(status_)) {
    parameters_->csetParameters(0.0, p, t, state_phase::GAS);
    return parameters_->cgetVolume();
  }
  return 0.0;
}

double NG_Gost::GetPressure(double v, double t) {
  (void)v;
  (void)t;
  Logging::Append(ERROR_CALC_MODEL_ST, "invalid operation for gost model");
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
