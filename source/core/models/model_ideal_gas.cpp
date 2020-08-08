/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#include "model_ideal_gas.h"

#include "gas_description_dynamic.h"
#include "gasmix_init.h"
#include "Logging.h"
#include "models_math.h"

#include <cmath>
#include <memory>

/** \brief строка model_info для идеального газа */
static model_str ideal_gas_mi(rg_model_id(rg_model_t::IDEAL_GAS,
    MODEL_SUBTYPE_DEFAULT), 1, 0, "Идеальный газ");

static model_priority ideal_gas_priority(DEF_PRIOR_IDEAL_GAS);

Ideal_Gas::Ideal_Gas(const model_input &mi)
  : modelGeneral(mi.ms, mi.gm, mi.bp) {
  set_gasparameters(mi.gpi, this);
  if (!error_.GetErrorCode())
    set_enthalpy();
  if (mi.mpri.IsSpecified()) {
    priority_ = mi.mpri;
  } else {
    priority_ = ideal_gas_priority;
  }
}

Ideal_Gas *Ideal_Gas::Init(const model_input &mi) {
  try {
    check_input(mi);
  } catch (const model_init_exception &e) {
    Logging::Append(e.what());
    return nullptr;
  }
  Ideal_Gas *ig = new Ideal_Gas(mi);
  if (ig)
    if (ig->parameters_ == nullptr) {
      delete ig;
      ig = nullptr;
    }
  return ig;
}

model_str Ideal_Gas::GetModelShortInfo(const rg_model_id &) {
  return ideal_gas_mi;
}

model_str Ideal_Gas::GetModelShortInfo() const {
  return ideal_gas_mi;
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

bool Ideal_Gas::IsValid(parameters pars) const {
  // модель идеального газа на практике особо
  //   не применяется, так что всегда можно говорить да
  return true;
}

void Ideal_Gas::SetVolume(double p, double t) {
  set_parameters(GetVolume(p, t), p, t);
}

void Ideal_Gas::SetPressure(double v, double t) {
  set_parameters(v, GetPressure(v, t), t);
}

double Ideal_Gas::GetVolume(double p, double t) {
  if (!is_above0(p, t)) {
    error_.SetError(ERROR_PAIR_DEFAULT(ERROR_CALC_MODEL_ST));
    return 0.0;
  }
  return  t * parameters_->cgetR() / p;
}

double Ideal_Gas::GetPressure(double v, double t) {
  if (!is_above0(v, t)) {
    error_.SetError(ERROR_PAIR_DEFAULT(ERROR_CALC_MODEL_ST));
    return 0.0;
  }
  return t * parameters_->cgetR() / v;
}
