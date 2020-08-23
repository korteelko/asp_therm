/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#include "models_creator.h"

#include "gas_by_file.h"
#include "gasmix_by_file.h"
#include "model_ideal_gas.h"
#include "model_ng_gost.h"
#include "model_peng_robinson.h"
#include "model_redlich_kwong.h"
#include "model_redlich_kwong_soave.h"
#include "models_configurations.h"

#include <functional>
#include <map>
#include <string>
#include <vector>
#include <utility>

#include <assert.h>


/** \brief Стандартные физические условия */
static parameters standard_conds = {0, 100000.0, 314.0};

/* todo:
 *   добавить такую же функцию в класс ProgramState */
model_input ModelsCreator::set_input(model_str ms, binodalpoints *bp,
    double p, double t, const parameters_mix &mix_components) {
  gas_marks_t gm = 0x00;
  rg_model_t mn = ms.model_type.type;
  gm = (uint32_t)mn | ((uint32_t)mn << BINODAL_MODEL_SHIFT);
  AddGasMixMark(&gm);
  model_input &&mi = model_input(gm, bp, {p, t,
      const_dyn_union{.components = &mix_components}}, ms);
  return std::move(mi);
}

model_input ModelsCreator::set_input(model_str ms, binodalpoints *bp,
    double p, double t, const ng_gost_mix &mix_components) {
  gas_marks_t gm = 0x00;
  rg_model_t mn = ms.model_type.type;
  gm = (uint32_t)mn | ((uint32_t)mn << BINODAL_MODEL_SHIFT);
  AddGostModelMark(&gm);
  model_input &&mi = model_input(gm, bp, {p, t,
      const_dyn_union{.ng_gost_components = &mix_components}}, ms);
  return std::move(mi);
}

modelGeneral *ModelsCreator::GetCalculatingModel(model_str ms,
    std::vector<gasmix_component_info> components, double p, double t) {
  /* todo: remove implementation! */
  #ifdef _DEBUG
  std::unique_ptr<GasMixByFiles<XMLReader>> gm(
      GasMixByFiles<XMLReader>::Init(components));
  #endif  // _DEBUG
  return (gm) ? getModel(ms, gm.get(), p, t) : nullptr;
}

modelGeneral *ModelsCreator::GetCalculatingModel(model_str ms,
    std::vector<gasmix_component_info> components) {
  return ModelsCreator::GetCalculatingModel(ms, components,
      standard_conds.pressure, standard_conds.temperature);
}

modelGeneral *ModelsCreator::GetCalculatingModel(model_str ms,
    file_utils::FileURLRoot *root_dir, const std::string &gasmix_xml,
    double p, double t) {
  // zdesya
  // nujno uchitivat root directory
  std::unique_ptr<GasMixComponentsFile<XMLReader>> gm(
      GasMixComponentsFile<XMLReader>::Init(
      ms.model_type.type, root_dir, gasmix_xml));
  return (gm) ? getModel(ms, gm.get(), p, t) : nullptr;
}

modelGeneral *ModelsCreator::GetCalculatingModel(model_str ms,
    file_utils::FileURLRoot *root_dir, const std::string &gasmix_xml) {
  return ModelsCreator::GetCalculatingModel(ms, root_dir, gasmix_xml,
      standard_conds.pressure, standard_conds.temperature);
}
modelGeneral *ModelsCreator::GetCalculatingModel(model_str ms,
    const ng_gost_mix &ngg, double p, double t) {
  return initModel(ms, nullptr, p,
      t, const_dyn_union{.ng_gost_components = &ngg});
}

modelGeneral *ModelsCreator::GetCalculatingModel(model_str ms,
    const ng_gost_mix &ngg) {
  return ModelsCreator::GetCalculatingModel(ms, ngg,
      standard_conds.pressure, standard_conds.temperature);
}

modelGeneral *ModelsCreator::initModel(model_str ms, binodalpoints *bp,
    double p, double t, const_dyn_union cdu) {
  modelGeneral *mg = nullptr;
  switch (ms.model_type.type) {
    case rg_model_t::IDEAL_GAS:
      mg = Ideal_Gas::Init(set_input(ms, bp, p, t, *cdu.components));
      break;
    case rg_model_t::REDLICH_KWONG:
      if (ms.model_type.subtype == MODEL_RK_SUBTYPE_SOAVE)
        mg = Redlich_Kwong_Soave::Init(
            set_input(ms, bp, p, t, *cdu.components));
      else
        mg = Redlich_Kwong2::Init(set_input(ms, bp, p, t, *cdu.components));
      break;
    case rg_model_t::PENG_ROBINSON:
      mg = Peng_Robinson::Init(set_input(ms, bp, p, t, *cdu.components));
      break;
    case rg_model_t::NG_GOST:
      mg = NG_Gost::Init(set_input(ms, bp, p, t, *cdu.ng_gost_components));
      break;
    case rg_model_t::EMPTY:
      break;
  }
  if (mg) {
    if (mg->GetError()) {
      delete mg;
      mg = nullptr;
    }
  }
  return mg;
}
/*
modelGeneral *Ideal_gas_equation::GetCalculatingModel(
    parameters prs, const_parameters cgp,
    dyn_parameters dgp, modelName mn = modelName::REDLICH_KWONG2) {
  // calculate binodal points
  reset_error();
  binodalpoints bp = PhaseDiagram::GetCalculated().GetBinodalPoints(
      cgp.V_K, cgp.P_K, cgp.T_K, mn, cgp.acentricfactor);
  // check calculated bp
  if (bp.p.empty()) {
    std::cerr << get_error_message();
    std::cerr << "\n Could not create Idealgas Solver\n" << std::endl;
    return nullptr;
  }
  return IdealGas::Init(mn, prs, cgp, dgp, bp);
}

modelGeneral *Ideal_gas_equation::GetCalculatingModel(
    parameters prs,
    parameters_mix &components, modelName mn = modelName::REDLICH_KWONG2) {
  reset_error();
  if (components.empty()) {
    std::cerr << " components of gas_mix was not setted!\n";
    return NULL;
  }
  binodalpoints bp = PhaseDiagram::GetCalculated().GetBinodalPoints(
      components, mn);
  if (bp.p.empty()) {
    std::cerr << get_error_message();
    std::cerr << "\n Could not create RedlichKwong2 Solver\n" << std::endl;
    return nullptr;
  }
  return IdealGas::Init(mn, prs, components, bp);
}
*/
/*  to another file

// Конфигурация расчёта, может изменяться
//   если файл кфг изменился, или ещё что
// Определены в subroutins/load_config.cpp
extern const double MODELS_DTIME;
extern const size_t MODELS_STEPS;

namespace {
  std::map<std::string, int32_t> equations {
      {"IDEAL_GAS", 1}, {"REDLICH_KWONG", 2}, {"PENG_ROBINSON", 3}};
}
//================================
// GasDynamic::getFlowCalculator
//================================

balloonFlowDynamic *
GasDynamic::getFlowCalculator(
                 const std::shared_ptr<modelGeneral> &spmg,
               gasparameters &outballoon, double V, double F) {
  if (spmg == nullptr) {
      std::cout << 
        "Object FlowCalculator wasn't created. Constructor get nullptr\n";
      return nullptr;
    }
  try {
    return new balloonFlowDynamic (spmg, outballoon, V, F);
  } catch (modelExceptions &e) {
    std::cout << " Object FlowCalculator wasn't created.\n" 
              << e.what() << std::endl;
  }
  return nullptr;
}

//================================
// GasDynamic::calculation
//================================

void GasDynamic::calculation(std::shared_ptr<InputData> &idp) {
  if (idp == nullptr) {
      std::cout << " Calculation method get nullptr to InputData";
      return;
    }
  idp_ = idp;
  std::shared_ptr<Equation_of_state> eos(setEOS());
  std::shared_ptr<constgasparameters> cgp(
      constgasparameters::getInstance(idp_->getConstgasparameters()));
  if (eos == nullptr) {
      std::cout << " Bad inputdata for Equation of state";
      return;
    }
  if (cgp == nullptr) {
      std::cout << " Bad inputdata for constgasparameters";
      return;
    }
  auto mg = std::shared_ptr<modelGeneral>(eos->getCalculatingModel(
        modelName::PENG_ROBINSON, cgp));
  if (mg == nullptr) {
      std::cout << " Bad inputdata for CalculatingModel";
      return;
    }
  std::shared_ptr<gasparameters> gp;
  std::unique_ptr<balloonFlowDynamic> up(setCalculator(mg, gp));
  if (up == nullptr) {
      std::cout << " Bad inputdata for setCalculator";
      return;
    }
  formatted_output SOout(std::cout);
  auto msgf = [] () { std::cout << " Bad inputdata for type of flow";};
  std::string InOutCheck = idp_->getFlowType();
  (InOutCheck == "IN") ? 
    up->calculateFlow<DerivateFunctorInflow>(MODELS_DTIME, MODELS_STEPS, SOout)
      :((InOutCheck == "OUT") ? 
         up->calculateFlow<DerivateFunctorOutflow>(MODELS_DTIME, MODELS_STEPS, SOout)
                          : msgf());
}

//================================
// GasDynamic::setCalculator
//================================

balloonFlowDynamic *GasDynamic::setCalculator(
                             std::shared_ptr<modelGeneral> &mg,
                                            std::shared_ptr<gasparameters> &pgp) {
  std::vector<double> params       = idp_->getBalloonParameters();
  std::pair<double, double> balloon = idp_->getBalloonVF();
  char func = idp_->getFunction()[0];
  try {
    switch (func) {
      case 'V': {
          mg->setVolume(params[1], params[2]);
          pgp = std::make_shared<gasparameters> (0.0, params[4], params[5], 
                                                     mg->getConstParameters());
          return getFlowCalculator(mg, *pgp, balloon.first, balloon.second);
        }
      case 'P': {
          mg->setPressure(params[0], params[2]);
          pgp = std::make_shared <gasparameters> (params[3], 0.0, params[5],
                                                    mg->getConstParameters());
          return getFlowCalculator(mg, *pgp, balloon.first, balloon.second);
        }
      default:
        std::cout << " Bad inputdata in initfile FUNCTION";
        return nullptr;
      }
  }
  catch(modelExceptions &e) {
    std::cout << e.what() << std::endl;
    return nullptr;
  }
} */
