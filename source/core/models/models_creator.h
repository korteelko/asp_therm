/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020-2021 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#ifndef _CORE__MODELS__MODELS_CREATOR_H_
#define _CORE__MODELS__MODELS_CREATOR_H_

#include "ErrorWrap.h"
#include "FileURL.h"
#include "gas_description.h"
#include "model_general.h"
#include "phase_diagram.h"

#include <exception>


/**
 * \brief Класс инициализации моделей
 * */
class ModelsCreator {
public:
  static modelGeneral *GetCalculatingModel(model_str ms,
      std::vector<gasmix_component_info> components, double p, double t);
  static modelGeneral *GetCalculatingModel(model_str ms,
      std::vector<gasmix_component_info> components);
  /**
   * \brief Инициализировать расчётную модель по файлу газовой смеси
   * \param ms Информация об инициализированной модели
   * \param root_di Указатель на корневую директорию, может быть равен nullptr
   * \param gasmix_xml Относительный путь к файлу смеси
   * \param p Давление
   * \param t Температура
   * */
  static modelGeneral *GetCalculatingModel(model_str ms,
      file_utils::FileURLRoot *root_dir, const std::string &gasmix_xml,
      double p, double t);
  /**
   * \brief Инициализировать расчётную модель по файлу газовой смеси
   * \param ms Информация об инициализированной модели
   * \param root_di Указатель на корневую директорию, может быть равен nullptr
   * \param gasmix_xml Относительный путь к файлу смеси
   * */
  static modelGeneral *GetCalculatingModel(model_str ms,
      file_utils::FileURLRoot *root_dir, const std::string &gasmix_xml);

  static modelGeneral *GetCalculatingModel(model_str ms,
      const ng_gost_mix &ngg, double p, double t);
  static modelGeneral *GetCalculatingModel(model_str ms,
      const ng_gost_mix &ngg);

private:
  static modelGeneral *set_model(const model_input &mi,
      double p, double t);
  // set gas_mix from xml files
  static model_input set_input(model_str ms, binodalpoints *bp,
      double p, double t, const parameters_mix &components);
  // set gas_mix by gost_defines
  static model_input set_input(model_str ms, binodalpoints *bp,
      double p, double t, const ng_gost_mix &components);

  template <class gasmix_t>
  static modelGeneral *getModel(model_str ms,
      gasmix_t *gm, double p, double t) {
    modelGeneral *model = nullptr;
    PhaseDiagram &pd = PhaseDiagram::GetCalculated();
    // if ng_gost by xml_list
    if (ms.model_type.type == rg_model_t::NG_GOST) {
      auto gost_mix = gm->GetGostMixParameters();
      if (!gost_mix->empty()) {
        model = initModel(ms, nullptr, p, t,
            const_dyn_union{.ng_gost_components = gost_mix.get()});
      }
    } else {
      std::shared_ptr<parameters_mix> prs_mix = gm->GetMixParameters();
      if (prs_mix != nullptr) {
        binodalpoints *bp = nullptr;
        if (PhaseDiagram::IsValidModel(ms.model_type)) {
          bp = pd.GetBinodalPoints(*prs_mix, ms.model_type);
        } else {
          bp = pd.GetBinodalPoints(*prs_mix,
              rg_model_id(rg_model_t::PENG_ROBINSON, MODEL_SUBTYPE_DEFAULT));
        }
        model = initModel(ms, bp, p, t,
           const_dyn_union{.components = prs_mix.get()});
      }
    }
    return model;
  }

//  static modelGeneral *initModel(rg_model_t mn, binodalpoints &bp,
//      double p, double t, const parameters_mix &components);
  static modelGeneral *initModel(model_str ms, binodalpoints *bp,
      double p, double t, const_dyn_union cdu);

};
#endif  // !_CORE__MODELS__MODELS_CREATOR_H_
