/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#ifndef _CORE__MODELS__MODELS_CREATOR_H_
#define _CORE__MODELS__MODELS_CREATOR_H_

#include "gas_description.h"
#include "model_general.h"
#include "phase_diagram.h"
#include "xml_reader.h"

class ModelsCreator {
/* TODO:
 *   заменить тип на ErrorWrap */
  static ErrorWrap error_;
  // for multithread
  // std::mutex_t

private:
  static modelGeneral *set_model(const model_input &mi,
      double p, double t);
  // set gas_mix from xml files
  static model_input set_input(rg_model_t mn, binodalpoints *bp,
      double p, double t, const parameters_mix &components);
  // set gas_mix by gost_defines
  static model_input set_input(rg_model_t mn, binodalpoints *bp,
      double p, double t, const ng_gost_mix &components);

  template <class gasmix_t>
  static modelGeneral *getModel(rg_model_t mn,
      gasmix_t *gm, double p, double t) {
    modelGeneral *model = nullptr;
    PhaseDiagram &pd = PhaseDiagram::GetCalculated();
    // for binodal available only RK2 and PR
    // todo
    rg_model_t binodal_mn = (mn == rg_model_t::PENG_ROBINSON) ?
        rg_model_t::PENG_ROBINSON : rg_model_t::REDLICH_KWONG2;
    try {
      // if ng_gost by xml_list
      if (mn == rg_model_t::NG_GOST) {
        auto gost_mix = gm->GetGostMixParameters();
        if (!gost_mix->empty()) {
          model = initModel(mn, nullptr, p, t,
              const_dyn_union{.ng_gost_components = gost_mix.get()});
        }
      } else {
        std::shared_ptr<parameters_mix> prs_mix = gm->GetMixParameters();
        if (prs_mix != nullptr) {
          binodalpoints *bp = pd.GetBinodalPoints(*prs_mix, binodal_mn);
          model = initModel(mn, bp, p, t,
              const_dyn_union{.components = prs_mix.get()});
        }
      }
    } catch (PhaseDiagram::PhaseDiagramException &) {
      return nullptr;
    } catch (...) {
      assert(0 && "check error message and and err_code");
      return nullptr;
    }
    return model;
  }

//  static modelGeneral *initModel(rg_model_t mn, binodalpoints &bp,
//      double p, double t, const parameters_mix &components);
  static modelGeneral *initModel(rg_model_t mn, binodalpoints *bp,
      double p, double t, const_dyn_union cdu);

public:
  static modelGeneral *GetCalculatingModel(rg_model_t mn,
      std::vector<gasmix_file> components, double p, double t);
  static modelGeneral *GetCalculatingModel(rg_model_t mn,
      std::vector<gasmix_file> components);
  static modelGeneral *GetCalculatingModel(rg_model_t mn,
      const std::string &gasmix_xml, double p, double t);
  static modelGeneral *GetCalculatingModel(rg_model_t mn,
      const std::string &gasmix_xml);
  static modelGeneral *GetCalculatingModel(rg_model_t mn,
      const ng_gost_mix &ngg, double p, double t);
  static modelGeneral *GetCalculatingModel(rg_model_t mn,
      const ng_gost_mix &ngg);
};
#endif  // !_CORE__MODELS__MODELS_CREATOR_H_
