#include "gas_by_file.h"

#include "models_errors.h"
#include "models_math.h"

#include <algorithm>
#include <exception>
#include <vector>

#include <assert.h>
#include <string.h>

// path len from section to subgroup
//   don't touch it
#define XML_PATHLEN_SUBGROUP   3
#define PNT_PARAMS_COUNT       3
#define CRIT_PNT_PARAMS_COUNT  4

/*
#define CP_VOLUME          0
#define CP_PRESSURE        1
#define CP_TEMPERATURE     2
#define CP_COMPRESS_FACTOR 3
*/

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

double get_val(std::vector<std::string> &vec, const std::string &valname,
    XMLReader<gas_node> *xml_doc) {
  std::string tmp_str = "";
  vec[XML_PATHLEN_SUBGROUP] = valname;
  xml_doc->GetValueByPath(vec, &tmp_str);
  double ans;
  try {
    ans = std::stod(tmp_str);
  } catch (std::invalid_argument &) {
    ans = 0.0;
  } catch (std::out_of_range &) {
    set_error_message(ERR_INIT_T, "xml argument out of range!");
    ans = -1.0;
  }
  return ans;
}
}  // unnamed namespace

// XmlFile
ComponentByFile::ComponentByFile(XMLReader<gas_node> *xml_doc)
  : xml_doc_(xml_doc), gas_name_(GAS_TYPE_UNDEFINED) {}

ComponentByFile *ComponentByFile::Init(const std::string &filename) {
  XMLReader<gas_node> *xml_doc = XMLReader<gas_node>::Init(filename);
  if (xml_doc == nullptr)
    return nullptr;
  return new ComponentByFile(xml_doc);
}

std::shared_ptr<const_parameters> ComponentByFile::GetConstParameters() {
  reset_error();
  if (xml_doc_ == nullptr)
    return nullptr;
  // critical point parameters
  double cp[CRIT_PNT_PARAMS_COUNT];
  double mol, af;
  std::vector<std::string> tmp_vec(
    critical_point_path);
  tmp_vec.push_back("");
  for (int i = 0; i < CRIT_PNT_PARAMS_COUNT; ++i)
    cp[i] = get_val(tmp_vec, point_names[i], xml_doc_);
  // cp[CP_PRESSURE] *= 1000000;
  // fuuuuuuuuuuuuuu
  tmp_vec[XML_PATHLEN_SUBGROUP - 1] =
      const_parameters_path[XML_PATHLEN_SUBGROUP - 1];
  mol = get_val(tmp_vec, "molec_mass", xml_doc_);
  af  = get_val(tmp_vec, "acentric", xml_doc_);
  set_gas_name();
  return std::shared_ptr<const_parameters>(const_parameters::Init(gas_name_,
      cp[0], cp[1], cp[2], cp[3], mol, af));
}

std::shared_ptr<dyn_parameters> ComponentByFile::GetDynParameters() {
  reset_error();
  if (xml_doc_ == nullptr)
    return nullptr;
  // dynamic point parameters
  double pnt[PNT_PARAMS_COUNT];
  double hcv, hcp, int_eng;
  std::vector<std::string> tmp_vec(
    dynamic_point_path);
  tmp_vec.push_back("");
  for (int i = 0; i < PNT_PARAMS_COUNT; ++i)
    pnt[i] = get_val(tmp_vec, point_names[i], xml_doc_);

  /* set calculating dynamic parameters */
  dyn_setup ds = 0x00;
  tmp_vec[XML_PATHLEN_SUBGROUP - 1] =
      dynamic_capacities_path[XML_PATHLEN_SUBGROUP - 1];
  if (is_above0(hcv = get_val(tmp_vec, "heat_cap_vol", xml_doc_)))
    ds |= DYNAMIC_HEAT_CAP_VOL;
  if (is_above0(hcp = get_val(tmp_vec, "heat_cap_pres", xml_doc_)))
    ds |= DYNAMIC_HEAT_CAP_PRES;
  tmp_vec[XML_PATHLEN_SUBGROUP - 1] =
      dynamic_potentials_path[XML_PATHLEN_SUBGROUP - 1];
  if (is_above0(int_eng = get_val(tmp_vec, "internal_energy", xml_doc_)))
    ds |= DYNAMIC_INTERNAL_ENERGY;
  return std::shared_ptr<dyn_parameters>(dyn_parameters::Init(
      ds, hcv, hcp, int_eng,parameters{pnt[0], pnt[1], pnt[2]}));
}

void ComponentByFile::set_gas_name() {
  gas_name_ = gas_by_name(xml_doc_->GetRootName());
}

ComponentByFile::~ComponentByFile() {
  if (xml_doc_ != nullptr)
    delete xml_doc_;
}
