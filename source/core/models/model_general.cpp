/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#include "model_general.h"

#include "common.h"
#include "gas_description_dynamic.h"
#include "gasmix_init.h"
#include "gas_ng_gost.h"
#include "models_math.h"
#include "ErrorWrap.h"

#include <algorithm>
#ifdef _DEBUG
#  include <iostream>
#endif  // _DEBUG


/** \brief ошибка ининициализации модели - на проверке входных данных,
  *   проверяется modelsCreator */
ErrorWrap modelGeneral::init_error;

DerivateFunctor::~DerivateFunctor() {}

model_input::model_input(gas_marks_t gm, binodalpoints *bp,
    gas_params_input gpi, calculation_configuration calc_config)
  : gm(gm), bp(bp), gpi(gpi), calc_config(calc_config) {}

modelGeneral::modelGeneral(calculation_configuration calc_config,
    gas_marks_t gm, binodalpoints *bp)
  : status_(STATUS_DEFAULT), calc_config_(calc_config),
    gm_(gm), parameters_(nullptr), bp_(bp) {}

modelGeneral::~modelGeneral() {}

void modelGeneral::set_parameters(double v, double p, double t) {
  parameters_->csetParameters(v, p, t, set_state_phase(v, p, t));
}

// расчиать паросодержание
double modelGeneral::vapor_part(int32_t index) {
  assert(index > bp_->vLeft.size());
  return 0.0;
}

int32_t modelGeneral::set_state_phasesub(double p) {
  return (bp_->p.end() - std::find_if(bp_->p.begin() + 1, bp_->p.end(),
      std::bind2nd(std::less_equal<double>(), p)));
}

state_phase modelGeneral::set_state_phase(
    double v, double p, double t) {
  /* todo: для газовых смесей наблюдается расслоение газовых фаз,
   *   так что задача не столь тривиальна */
  if (bp_ == nullptr)
    return state_phase::NOT_SET;
  if (t >= parameters_->cgetT_K())
    return (p >= parameters_->cgetP_K()) ? state_phase::SCF : state_phase::GAS;
  // if p on the left of binodal graph  -  liquid
  int32_t iter = set_state_phasesub(p);
  if (!iter) {
    return ((v <= parameters_->cgetV_K()) ?
        state_phase::LIQ_STEAM : state_phase::GAS);
  }
  iter = bp_->p.size() - iter - 1;
  assert(iter >= 0);
  /* calculate p)path in percents */
  const double p_path = (p - bp_->p[iter+1]) / (bp_->p[iter] - bp_->p[iter+1]);
  // left branch of binodal
  if (v < parameters_->cgetV_K()) {                 
    const double vapprox = bp_->vLeft[iter] - (bp_->vLeft[iter+1] -
        bp_->vLeft[iter]) * p_path;
    return ((v < vapprox) ? state_phase::LIQUID : state_phase::LIQ_STEAM);
  }
  // rigth branch of binodal
  const double vapprox = bp_->vRigth[iter] + (bp_->vRigth[iter+1] -
      bp_->vRigth[iter])*p_path;
  return ((v > vapprox) ? state_phase::GAS : state_phase::LIQ_STEAM);
}

void modelGeneral::set_enthalpy() {
  if (bp_ == nullptr)
    return;
  if (!bp_->hLeft.empty())
    bp_->hLeft.clear();
  for (size_t i = 0; i < bp_->vLeft.size(); ++i) {
    SetPressure(bp_->vLeft[i], bp_->t[i]);
    bp_->hLeft.push_back(parameters_->cgetIntEnergy() +
        bp_->p[i] * bp_->vLeft[i]);
  }
  if (!bp_->hRigth.empty())
    bp_->hRigth.clear();
  for (size_t i = 0; i < bp_->vRigth.size(); ++i) {
    SetPressure(bp_->vRigth[i], bp_->t[i]);
    bp_->hRigth.push_back(parameters_->cgetIntEnergy() +
        bp_->p[i] * bp_->vRigth[i]);
  }
}

/// return NULL or pointer to GasParameters
const GasParameters *modelGeneral::get_gasparameters() const {
  return parameters_.get();
}

void modelGeneral::set_gasparameters(const gas_params_input &gpi,
    modelGeneral *mg) {
  if (HasGostModelMark(gm_)) {
    parameters_ = std::unique_ptr<GasParameters>(
        GasParameters_NG_Gost_dyn::Init(gpi));
  } else if (HasGasMixMark(gm_)) {
    // todo: самое интересное здесь, т.к. для различных моделей
    //   реаализуется различный подход
    parameters_ = std::unique_ptr<GasParameters>(
        GasParameters_mix_dyn::Init(gpi, mg));
  } else {
    parameters_ = std::unique_ptr<GasParameters>(
        GasParameters_dyn::Init(gpi, mg));
  }
  if (parameters_ == nullptr) {
    GasParameters::init_error.LogIt();
    error_.SetError(ERROR_INIT_T, "error occurred while init gost model");
  }
}

void modelGeneral::ResetInitError() {
  init_error.Reset();
}

merror_t modelGeneral::check_input(const model_input &mi) {
  merror_t err = ERROR_SUCCESS_T;
  if (HasGasMixMark(mi.gm) && HasGostModelMark(mi.gm))
     err = modelGeneral::init_error.SetError(ERROR_INIT_T,
        "options GAS_MIX_MARK and GAS_NG_GOST_MARK not compatible\n");
  if ((!err) && HasGasMixMark(mi.gm)) {
    if (!mi.gpi.const_dyn.components->empty()) {
      // для проверки установленных доль
      double parts_sum = 0.0;
      std::for_each(mi.gpi.const_dyn.components->begin(),
          mi.gpi.const_dyn.components->end(),
          [&parts_sum] (const std::pair<const double, const_dyn_parameters> &x)
               { parts_sum += x.first; });
      if (parts_sum < (GASMIX_PERSENT_AVR - GASMIX_PERCENT_EPS) ||
          parts_sum > (GASMIX_PERSENT_AVR + GASMIX_PERCENT_EPS))
        err = modelGeneral::init_error.SetError(ERROR_INIT_T,
            "gasmix sum of parts != 100%\n");
    }
  }
  if ((!err) && HasGostModelMark(mi.gm)) {
    if (!mi.gpi.const_dyn.ng_gost_components->empty()) {
      // для проверки установленных доль
      double parts_sum = 0.0;
      std::for_each(mi.gpi.const_dyn.ng_gost_components->begin(),
          mi.gpi.const_dyn.ng_gost_components->end(),
          [&parts_sum] (const std::pair<gas_t, double> &x)
              { parts_sum += x.second; });
      if (parts_sum < (GASMIX_PERSENT_AVR - GASMIX_PERCENT_EPS) ||
          parts_sum > (GASMIX_PERSENT_AVR + GASMIX_PERCENT_EPS))
        err = modelGeneral::init_error.SetError(ERROR_INIT_T,
            "gasmix sum of parts != 100%\n");
    }
  }
  if (!(err || is_above0(mi.gpi.p, mi.gpi.t)))
    err = modelGeneral::init_error.SetError(ERROR_INIT_T | ERROR_INIT_ZERO_ST);
  return err;
}

double modelGeneral::GetVolume() const {
  return parameters_->cgetVolume();
}

double modelGeneral::GetPressure() const {
  return parameters_->cgetPressure();
}

double modelGeneral::GetTemperature() const {
  return parameters_->cgetTemperature();
}

double modelGeneral::GetCP() const {
  return parameters_->cgetCP();
}

double modelGeneral::GetAcentric() const {
  return parameters_->cgetAcentricFactor();
}

double modelGeneral::GetT_K() const {
  return parameters_->cgetT_K();
}

state_phase modelGeneral::GetState() const {
  return parameters_->cgetState();
}

parameters modelGeneral::GetParametersCopy() const {
  return parameters_->cgetParameters();
}

const_parameters modelGeneral::GetConstParameters() const {
  return parameters_->cgetConstparameters();
}

calculation_state_log modelGeneral::GetStateLog() const {
  dyn_parameters dps = parameters_->cgetDynParameters();
  // пока так
  return {dps, dps.internal_energy * dps.parm.pressure * dps.parm.volume, 
      stateToString[(uint32_t)parameters_->cgetState()]};
}

merror_t modelGeneral::GetErrorCode() const {
  return error_.GetErrorCode();
}

std::string modelGeneral::ParametersString() const {
  char str[256] = {0};
  auto prs  = parameters_->cgetParameters();
  auto dprs = parameters_->cgetDynParameters();
  sprintf(str, "%12.1f %8.4f %8.2f %8.2f %8.2f %8.2f %8.2f\n",
      prs.pressure, prs.volume, 1.0 / prs.volume, prs.temperature,
      dprs.heat_cap_vol, dprs.heat_cap_pres, dprs.internal_energy);
  return std::string(str);
}

std::string modelGeneral::ConstParametersString() const {
  char str[256] = {0};
  const_parameters &cprs = parameters_->const_params;
  sprintf(str, "  Critical pnt: p=%12.1f; v=%8.4f; t=%8.2f\n"
      "  Others: mol_m=%6.3f R=%8.3f ac_f=%6.4f\n", cprs.P_K, cprs.V_K,
      cprs.T_K, cprs.molecularmass, cprs.R, cprs.acentricfactor);
  return std::string(str);
}

std::string modelGeneral::sParametersStringHead() {
  return "   pressure    volume   density  temperat   cv       cp       u\n";
}
