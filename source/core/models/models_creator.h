#ifndef _CORE__MODELS__MODELS_CREATOR_H_
#define _CORE__MODELS__MODELS_CREATOR_H_

#include "gas_description.h"
#include "model_general.h"
#include "model_ideal_gas.h"
#include "model_redlich_kwong.h"
#include "model_peng_robinson.h"
#include "phase_diagram.h"
#include "xml_reader.h"

class ModelsCreator {
  static modelGeneral *set_model(const model_input &mi,
      double p, double t);
  // set gas_mix from xml files
  static model_input set_input(rg_model_t mn, const binodalpoints &bp,
      double p, double t, const parameters_mix &components);
  // set gas_mix by gost_defines
  static model_input set_input(rg_model_t mn, const binodalpoints &bp,
      double p, double t, const ng_gost_mix &components);

  template <class GASMIX_T>
  static modelGeneral* getModel(rg_model_t mn,
      GASMIX_T *gm, double p, double t) {
    if (gm == nullptr)
      return nullptr;
    std::shared_ptr<parameters_mix> prs_mix = gm->GetParameters();
    if (prs_mix == nullptr)
      return nullptr;
    PhaseDiagram &pd = PhaseDiagram::GetCalculated();
    // for binodal available only RK2 and PR
    rg_model_t binodal_mn = (mn == rg_model_t::PENG_ROBINSON) ?
        rg_model_t::PENG_ROBINSON : rg_model_t::REDLICH_KWONG2;
    binodalpoints bp = pd.GetBinodalPoints(*prs_mix, binodal_mn);
    switch (mn) {
      case rg_model_t::IDEAL_GAS:
        return Ideal_Gas::Init(set_input(mn, bp, p, t, *prs_mix));
      case rg_model_t::REDLICH_KWONG2:
        return Redlich_Kwong2::Init(set_input(mn, bp, p, t, *prs_mix));
      case rg_model_t::PENG_ROBINSON:
        return Peng_Robinson::Init(set_input(mn, bp, p, t, *prs_mix));
      default:
        set_error_message(ERR_INIT_T, "undefined calculation model");
    }
    return nullptr;
  }

public:
  static modelGeneral *GetCalculatingModel(rg_model_t mn,
      std::vector<gasmix_file> components, double p, double t);
  static modelGeneral *GetCalculatingModel(rg_model_t mn,
      std::vector<gasmix_file> components);
  static modelGeneral *GetCalculatingModel(rg_model_t mn,
      const std::string &gasmix_xml, double p, double t);
  static modelGeneral *GetCalculatingModel(rg_model_t mn,
      const std::string &gasmix_xml);
};
#endif  // !_CORE__MODELS__MODELS_CREATOR_H_
