/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#ifndef _CORE__SUBROUTINS__GAS_BY_FILE_H_
#define _CORE__SUBROUTINS__GAS_BY_FILE_H_

#include "common.h"
#include "file_structs.h"
#include "gas_description.h"
#include "gasmix_init.h"
#include "models_math.h"

#include <memory>
#include <string>


#define XML_PATHLEN_SUBGROUP   3
#define PNT_PARAMS_COUNT       3
#define CRIT_PNT_PARAMS_COUNT  4

// statndart paths
namespace {
std::vector<std::string> critical_point_path {
  "parameters", "constants", "critical_point"};
std::vector<std::string> const_parameters_path {
  "parameters", "constants", "const_parameters"};
std::vector<std::string> dynamic_point_path {
  "parameters", "dynamics", "point"};
std::vector<std::string> dynamic_capacities_path {
  "parameters", "dynamics", "capacities"};
std::vector<std::string> dynamic_potentials_path {
  "parameters", "dynamics", "potentials"};

const std::string point_names[CRIT_PNT_PARAMS_COUNT] = {
    "volume", "pressure", "temperature", "compress_factor"};

}  // anonymous namespace

/* taks maybe add json format */
/** \brief шаблоный класс инициализации компонента
  *   газовой смеси, описанного в xml или json файле */
template <template<class gas_node> class ConfigReader>
class ComponentByFile {
  ComponentByFile(const ComponentByFile &) = delete;
  ComponentByFile &operator=(const ComponentByFile &) = delete;

public:
  static ComponentByFile *Init(const std::string &filename) {
    ConfigReader<gas_node> *xml_doc = ConfigReader<gas_node>::Init(filename);
    if (xml_doc == nullptr)
      return nullptr;
    return new ComponentByFile(xml_doc);
  }

  std::shared_ptr<const_parameters> GetConstParameters() {
   if (xml_doc_ == nullptr)
      return nullptr;
    // critical point parameters
    double cp[CRIT_PNT_PARAMS_COUNT];
    double mol, af;
    std::vector<std::string> tmp_vec(
      critical_point_path);
    tmp_vec.push_back("");
    for (int i = 0; i < CRIT_PNT_PARAMS_COUNT; ++i)
      cp[i] = get_val(tmp_vec, point_names[i], xml_doc_.get());
    // cp[CP_PRESSURE] *= 1000000;
    // fuuuuuuuuuuuuuu
    tmp_vec[XML_PATHLEN_SUBGROUP - 1] =
        const_parameters_path[XML_PATHLEN_SUBGROUP - 1];
    mol = get_val(tmp_vec, "molec_mass", xml_doc_.get());
    af  = get_val(tmp_vec, "acentric", xml_doc_.get());
    set_gas_name();
    return std::shared_ptr<const_parameters>(const_parameters::Init(
        gas_name_, cp[0], cp[1], cp[2], cp[3], mol, af));
  }

  std::shared_ptr<dyn_parameters> GetDynParameters() {
    if (xml_doc_ == nullptr)
      return nullptr;
    // dynamic point parameters
    double pnt[PNT_PARAMS_COUNT];
    double hcv, hcp, int_eng;
    std::vector<std::string> tmp_vec(
        dynamic_point_path);
    tmp_vec.push_back("");
    for (int i = 0; i < PNT_PARAMS_COUNT; ++i)
      pnt[i] = get_val(tmp_vec, point_names[i], xml_doc_.get());

    /* set calculating dynamic parameters */
    dyn_setup ds = 0x00;
    tmp_vec[XML_PATHLEN_SUBGROUP - 1] =
        dynamic_capacities_path[XML_PATHLEN_SUBGROUP - 1];
    if (is_above0(hcv = get_val(tmp_vec, "heat_cap_vol", xml_doc_.get())))
      ds |= DYNAMIC_HEAT_CAP_VOL;
    if (is_above0(hcp = get_val(tmp_vec, "heat_cap_pres", xml_doc_.get())))
      ds |= DYNAMIC_HEAT_CAP_PRES;
    tmp_vec[XML_PATHLEN_SUBGROUP - 1] =
        dynamic_potentials_path[XML_PATHLEN_SUBGROUP - 1];
    if (is_above0(int_eng = get_val(tmp_vec, "internal_energy", xml_doc_.get())))
      ds |= DYNAMIC_INTERNAL_ENERGY;
    return std::shared_ptr<dyn_parameters>(dyn_parameters::Init(
        ds, hcv, hcp, int_eng,parameters{pnt[0], pnt[1], pnt[2]}));
  }

private:
  ComponentByFile(ConfigReader<gas_node> *xml_doc)
    : status_(STATUS_DEFAULT), xml_doc_(xml_doc),
      gas_name_(GAS_TYPE_UNDEFINED) {}

  double get_val(std::vector<std::string> &vec, const std::string &valname,
      const ConfigReader<gas_node> *config_doc) {
    std::string tmp_str = "";
    vec[XML_PATHLEN_SUBGROUP] = valname;
    config_doc->GetValueByPath(vec, &tmp_str);
    double ans;
    try {
      ans = std::stod(tmp_str);
    } catch (std::invalid_argument &) {
      ans = 0.0;
    } catch (std::out_of_range &) {
      ans = -1.0;
    }
    return ans;
  }

  void set_gas_name() {
    gas_name_ = gas_by_name(xml_doc_->GetRootName());
  }

private:
  mstatus_t status_;
  std::unique_ptr<ConfigReader<gas_node>> xml_doc_;
  gas_t gas_name_;
};

#endif  // !_CORE__SUBROUTINS__GAS_BY_FILE_H_
