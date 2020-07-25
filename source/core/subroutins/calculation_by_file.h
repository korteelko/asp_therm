/**
 * asp_therm - implementation of real gas equations of state
 * ===================================================================
 * * calculation_byfile *
 *   В файле описан шаблон класса, предоставляющего функционал
 * инициализации конфигурации расчётов из текстового файла.
 * ===================================================================
 *
 * Copyright (c) 2020 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#ifndef _CORE__SUBROUTINS__CALCULATION_BY_FILE_H_
#define _CORE__SUBROUTINS__CALCULATION_BY_FILE_H_

#include "calculation_info.h"
#include "Common.h"
#include "configuration_strtpl.h"
#include "ErrorWrap.h"
#include "INode.h"
#include "Reader.h"


/*
<calc_setup name="example">
  <models> PRb, GOST, ISO </models>
  <gasmix_files>
    <mixfile name="1"> gost/gostmix1.xml </mixfile>
    <mixfile name="2"> gost/gostmix2.xml </mixfile>
    <mixfile name="3"> gost/gostmix3.xml </mixfile>
  </gasmix_files>
  <points>
    <point p="10000" t="250.0"/>
    <point p="10000" t="300.0"/>
    <point p="10000" t="350.0"/>
  </points>
</calc_setup>
*/
template <class ReaderNodeT>
class calculation_node;
/** \brief Класс собирающий сетап расчёта */
class CalculationBuilder {
  friend class calculation_node<pugi::xml_node>;
  // otsuda
public:
  CalculationBuilder(calculation_setup *setup_p)
    : setup_p(setup_p) {}

  template <class ReaderNodeT>
  calculation_node<ReaderNodeT> *GetNodeInitializer() {
    return new calculation_node<ReaderNodeT>(this);
  }

protected:
  ErrorWrap error;
  /** \brief Указатель на инициализируемую структуру */
  calculation_setup *setup_p = nullptr;
};

/** \brief Структура узла конфигурации расчёте */
template <class ReaderNodeT>
class calculation_node: public INodeInitializer {
  static std::map<std::string, rg_model_id> models_ids;

public:
  calculation_node()
    : builder(nullptr) {}
  calculation_node(CalculationBuilder *builder)
    : builder(builder) {}
  void SetSubnodesNames(inodes_vec *subnodes) override {
    subnodes->clear();
    subnodes->insert(subnodes->end(), subnodes_.cbegin(), subnodes_.cend());
  }
  bool IsLeafNode() const override {
    return !have_subnodes_;
  }
  merror_t InitData(pugi::xml_node *src, const std::string &nodename){
    if (!src)
      return ERROR_INIT_NULLP_ST;
    name_ = nodename;
    source = src;
    set_subnodes();
    if (name_ == STRTPL_CALCUL_MODELS) {
      set_models();
    } else if (name_ == STRTPL_CALCUL_GASMIX) {
      set_mixfiles();
    } else if (name_ == STRTPL_CALCUL_POINTS) {
      set_points();
    }
    return ERROR_SUCCESS_T;
  }

protected:
  std::string getParameter(const std::string &name) override {
    std::string val = "";
    auto p = source->child(name.c_str());
    if (!p.empty())
      val = p.first_child().value();
    return val;
  }

  /* data initialization */
  void set_subnodes() {
    subnodes_.clear();
    have_subnodes_ = false;
    if (name_ == STRTPL_CALCUL_ROOT) {
      have_subnodes_ = true;
      subnodes_.push_back(STRTPL_CALCUL_MODELS);
      subnodes_.push_back(STRTPL_CALCUL_GASMIX);
      subnodes_.push_back(STRTPL_CALCUL_POINTS);
    }
  }
  /* init source structures */
  void set_models() {
    std::string models_str(source->first_child().value());
    std::vector<std::string> models_vec;
    split_str(models_str, &models_vec, ',');
    for (const auto &mstr: models_vec) {
      auto it = models_ids.find(trim_str(mstr));
      if (it != models_ids.end())
        builder->setup_p->models.push_back(it->second);
    }
  }
  void set_mixfiles() {
    for (auto x: source->children())
      builder->setup_p->gasmix_files.push_back(
          builder->setup_p->root->CreateFileURL(x.value()));
  }
  void set_points() {
    for (auto x: source->children()) {
      try {
        parameters p;
        p.volume = 0.0;
        p.pressure = std::stod(x.attribute(STRTPL_CALCUL_P).value());
        p.temperature = std::stod(x.attribute(STRTPL_CALCUL_T).value());
        builder->setup_p->points.push_back(p);
      } catch (std::exception &e) {
        builder->error.SetError(ERROR_PAIR_DEFAULT(ERROR_READER_PARSE_ST));
      }
    }
  }

public:
  /** \brief Указатель на соответствующий узел pugi */
  ReaderNodeT *source = nullptr;
  /** \brief Указатель на инициализатор, хранилище данных */
  CalculationBuilder *builder = nullptr;
  bool have_subnodes_ = false;
};

template <class ReaderNodeT>
std::map<std::string, rg_model_id> calculation_node<ReaderNodeT>::models_ids {
  {"IG", rg_model_id(rg_model_t::IDEAL_GAS, MODEL_SUBTYPE_DEFAULT)},
  {"RK", rg_model_id(rg_model_t::REDLICH_KWONG, MODEL_SUBTYPE_DEFAULT)},
  {"SRK", rg_model_id(rg_model_t::REDLICH_KWONG, MODEL_RK_SUBTYPE_SOAVE)},
  {"PR", rg_model_id(rg_model_t::PENG_ROBINSON, MODEL_SUBTYPE_DEFAULT)},
  {"PRb", rg_model_id(rg_model_t::PENG_ROBINSON, MODEL_PR_SUBTYPE_BINASSOC)},
  {"GOST", rg_model_id(rg_model_t::NG_GOST, MODEL_SUBTYPE_DEFAULT)},
  {"ISO", rg_model_id(rg_model_t::NG_GOST, MODEL_GOST_SUBTYPE_ISO_20765)},
};


#endif  // !_CORE__SUBROUTINS__CALCULATION_BY_FILE_H_
