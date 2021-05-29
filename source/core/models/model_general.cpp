/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020-2021 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#include "model_general.h"

#include "gas_description_dynamic.h"
#include "gas_ng_gost30319.h"
#include "model_ideal_gas.h"
#include "model_ng_gost.h"
#include "model_peng_robinson.h"
#include "model_redlich_kwong.h"
#include "models_math.h"

#include <algorithm>
#ifdef _DEBUG
#include <iostream>
#endif  // _DEBUG

DerivateFunctor::~DerivateFunctor() {}

model_input::model_input(gas_marks_t gm,
                         binodalpoints* bp,
                         gas_params_input gpi,
                         model_str ms)
    : gm(gm), bp(bp), gpi(gpi), ms(ms) {}

/* model_init_exception */
modelGeneral::model_init_exception::model_init_exception(
    const model_str* info,
    const std::string& _msg)
    : msg(_msg) {
  if (info)
    msg += "\n\tМодель:" + info->GetString();
}

const char* modelGeneral::model_init_exception::what() const noexcept {
  return msg.c_str();
}

/* modelGeneral */
modelGeneral::modelGeneral(model_str model_config,
                           gas_marks_t gm,
                           binodalpoints* bp)
    : status_(STATUS_DEFAULT),
      model_config_(model_config),
      gm_(gm),
      parameters_(nullptr),
      bp_(bp) {}

modelGeneral::~modelGeneral() {}

void modelGeneral::SetCalculationSetup(calculation_info* calculation) {
  calculation_ = calculation;
}

void modelGeneral::set_parameters(double v, double p, double t) {
  parameters_->csetParameters(v, p, t, set_state_phase(v, p, t));
}

// расчиать паросодержание
double modelGeneral::vapor_part(int32_t index) {
  assert(index > bp_->vLeft.size());
  return 0.0;
}

int32_t modelGeneral::set_state_phasesub(double p) {
  return (bp_->p.end()
          - std::find_if(bp_->p.begin() + 1, bp_->p.end(),
                         [p](const auto v) { return v < p; }));
}

state_phase modelGeneral::set_state_phase(double v, double p, double t) {
  /* todo: для газовых смесей наблюдается расслоение газовых фаз,
   *   так что задача не столь тривиальна */
  if (bp_ == nullptr)
    return state_phase::NOT_SET;
  if (t >= parameters_->cgetT_K())
    return (p >= parameters_->cgetP_K()) ? state_phase::SCF : state_phase::GAS;
  // if p on the left of binodal graph  -  liquid
  int32_t iter = set_state_phasesub(p);
  if (!iter) {
    return ((v <= parameters_->cgetV_K()) ? state_phase::LIQ_STEAM
                                          : state_phase::GAS);
  }
  iter = bp_->p.size() - iter - 1;
  assert(iter >= 0);
  /* calculate p)path in percents */
  const double p_path =
      (p - bp_->p[iter + 1]) / (bp_->p[iter] - bp_->p[iter + 1]);
  // left branch of binodal
  if (v < parameters_->cgetV_K()) {
    const double vapprox =
        bp_->vLeft[iter] - (bp_->vLeft[iter + 1] - bp_->vLeft[iter]) * p_path;
    return ((v < vapprox) ? state_phase::LIQUID : state_phase::LIQ_STEAM);
  }
  // rigth branch of binodal
  const double vapprox =
      bp_->vRigth[iter] + (bp_->vRigth[iter + 1] - bp_->vRigth[iter]) * p_path;
  return ((v > vapprox) ? state_phase::GAS : state_phase::LIQ_STEAM);
}

void modelGeneral::set_enthalpy() {
  if (bp_ == nullptr)
    return;
  if (!bp_->hLeft.empty())
    bp_->hLeft.clear();
  for (size_t i = 0; i < bp_->vLeft.size(); ++i) {
    SetPressure(bp_->vLeft[i], bp_->t[i]);
    bp_->hLeft.push_back(parameters_->cgetIntEnergy()
                         + bp_->p[i] * bp_->vLeft[i]);
  }
  if (!bp_->hRigth.empty())
    bp_->hRigth.clear();
  for (size_t i = 0; i < bp_->vRigth.size(); ++i) {
    SetPressure(bp_->vRigth[i], bp_->t[i]);
    bp_->hRigth.push_back(parameters_->cgetIntEnergy()
                          + bp_->p[i] * bp_->vRigth[i]);
  }
}

/// return NULL or pointer to GasParameters
const GasParameters* modelGeneral::get_gasparameters() const {
  return parameters_.get();
}

void modelGeneral::set_gasparameters(const gas_params_input& gpi,
                                     modelGeneral* mg) {
  if (HasGostModelMark(gm_)) {
    parameters_ = std::unique_ptr<GasParameters>(
        GasParametersGost30319Dyn::Init(gpi, HasGostISO20765Mark(gm_)));
  } else if (HasGasMixMark(gm_)) {
    // todo: самое интересное здесь, т.к. для различных моделей
    //   реаализуется различный подход
    parameters_ =
        std::unique_ptr<GasParameters>(GasParameters_mix_dyn::Init(gpi, mg));
  } else {
    parameters_ =
        std::unique_ptr<GasParameters>(GasParameters_dyn::Init(gpi, mg));
  }
  if (parameters_ == nullptr) {
    error_.SetError(ERROR_INIT_T, "error occurred while init gost model");
    status_ = STATUS_HAVE_ERROR;
  }
}

model_str modelGeneral::GetModelShortInfo(const rg_model_id model_type) {
  switch (model_type.type) {
    case rg_model_t::IDEAL_GAS:
      return Ideal_Gas::GetModelShortInfo(model_type);
    case rg_model_t::REDLICH_KWONG:
      // На модификацию Соаве тоже получаем отсюда
      return Redlich_Kwong2::GetModelShortInfo(model_type);
    case rg_model_t::PENG_ROBINSON:
      return Peng_Robinson::GetModelShortInfo(model_type);
    case rg_model_t::NG_GOST:
      return NG_Gost::GetModelShortInfo(model_type);
    default:
      break;
  }
  return model_str(rg_model_id(rg_model_t::EMPTY, MODEL_SUBTYPE_DEFAULT), 1, 0,
                   "");
}

void modelGeneral::check_input(const model_input& mi) {
  // Проверка флагов
  if (HasGasMixMark(mi.gm) && HasGostModelMark(mi.gm))
    throw modelGeneral::model_init_exception(
        &mi.ms, "options GAS_MIX_MARK and GAS_NG_GOST_MARK not compatible\n");
  // Проверка установленных доль
  float perc = modelGeneral::calculate_parts_sum(mi);
  if (!is_equal(perc, GASMIX_PERSENT_AVR, GASMIX_PERCENT_EPS))
    throw modelGeneral::model_init_exception(
        &mi.ms,
        "gasmix sum of parts != 100% -> " + std::to_string(perc) + "\n");
  // Проверка стартовых параметров
  if (!is_above0(mi.gpi.p, mi.gpi.t))
    throw modelGeneral::model_init_exception(&mi.ms, ERROR_INIT_ZERO_ST_MSG);
}

double modelGeneral::calculate_parts_sum(const model_input& mi) {
  double parts_sum = 0.0;
  if (HasGasMixMark(mi.gm)) {
    if (!mi.gpi.const_dyn.components->empty()) {
      std::for_each(
          mi.gpi.const_dyn.components->begin(),
          mi.gpi.const_dyn.components->end(),
          [&parts_sum](const std::pair<const double, const_dyn_parameters>& x) {
            parts_sum += x.first;
          });
    }
  } else if (HasGostModelMark(mi.gm)) {
    if (!mi.gpi.const_dyn.ng_gost_components->empty()) {
      std::for_each(mi.gpi.const_dyn.ng_gost_components->begin(),
                    mi.gpi.const_dyn.ng_gost_components->end(),
                    [&parts_sum](const std::pair<gas_t, double>& x) {
                      parts_sum += x.second;
                    });
    }
  } else {
    // смеси нет - только один компонент
    parts_sum = 1.0;
  }
  return parts_sum;
}

priority_var modelGeneral::GetPriority() const {
  return priority_.GetPriority();
}

const model_str* modelGeneral::GetModelConfig() const {
  return &model_config_;
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

// todo: ??? насчёт энтальпии, вн. энергии и т.д. не совсем прозрачно
calculation_state_log modelGeneral::GetStateLog() const {
  calculation_state_log s = {
      .id = -1,
      .calculation = calculation_,
      .state_phase = stateToString[(uint32_t)parameters_->cgetState()]};
  s.initialized |= calculation_state_log::f_state_phase;
  s.SetDynPars(parameters_->cgetDynParameters());
  return s;
}

merror_t modelGeneral::GetError() const {
  return error_.GetErrorCode();
}

calculation_info* modelGeneral::GetCalculationInfo() const {
  return calculation_;
}

std::string modelGeneral::ParametersString() const {
  // todo: remove to class GasParamaters
  char str[256] = {0};
  auto prs = parameters_->cgetParameters();
  auto dprs = parameters_->cgetDynParameters();
  snprintf(str, sizeof(str) - 1, "%12.1f %8.4f %8.2f %8.2f %8.2f %8.2f %8.2f\n",
           prs.pressure, prs.volume, 1.0 / prs.volume, prs.temperature,
           dprs.heat_cap_vol, dprs.heat_cap_pres, dprs.internal_energy);
  return std::string(str);
}

std::string modelGeneral::ConstParametersString(std::string pref) const {
  // todo: передавать вызов нужно к parameters_, а не parameters_->const_params
  return parameters_->const_params.GetString(pref);
}

std::string modelGeneral::sParametersStringHead() {
  return "   pressure    volume   density  temperat   cv       cp       u\n";
}
