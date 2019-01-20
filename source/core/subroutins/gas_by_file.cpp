#include "gas_by_file.h"

#include "models_errors.h"
#include "models_math.h"

#include <algorithm>
#include <map>

#include <assert.h>
#include <string.h>

// path len from section to subgroup
#define XML_PATHLEN_SUBGROUP 3
#define CP_PARAMETERS_COUNT  3

#define CP_VOLUME       0
#define CP_PRESSURE     1
#define CP_TEMPERATURE  2

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

const std::string cp_names[CP_PARAMETERS_COUNT] = {
    "volume", "pressure", "temperature"};

double get_val(std::vector<std::string> &vec, const std::string &valname,
    XMLReader<gas_node> *xml_doc) {
  std::string tmp_str = "";
  vec[XML_PATHLEN_SUBGROUP] = valname;
  xml_doc->GetValueByPath(vec, &tmp_str);
  return std::stod(tmp_str);
}

std::map<std::string, gas_t> gas_names = std::map<std::string, gas_t> {
  {"methane", GAS_TYPE_METHANE},
  {"ethane", GAS_TYPE_ETHANE},
  {"propane", GAS_TYPE_PROPANE}
};
// не оч точная функция
double get_intern_energy(double cv, double temper) {
  return cv * temper;
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
  double cp[CP_PARAMETERS_COUNT];
  double mol, af;
  std::vector<std::string> tmp_vec(
    critical_point_path);
  tmp_vec.push_back("");
  for (int i = 0; i < CP_PARAMETERS_COUNT; ++i)
    cp[i] = get_val(tmp_vec, cp_names[i], xml_doc_);
  // cp[CP_PRESSURE] *= 1000000;
  // fuuuuuuuuuuuuuu
  tmp_vec[XML_PATHLEN_SUBGROUP - 1] =
      const_parameters_path[XML_PATHLEN_SUBGROUP - 1];
  mol = get_val(tmp_vec, "molec_mass", xml_doc_);
  af  = get_val(tmp_vec, "acentric", xml_doc_);
  set_gas_name();
  return std::shared_ptr<const_parameters>(const_parameters::Init(gas_name_,
      cp[0], cp[1], cp[2], mol, af));
}

std::shared_ptr<dyn_parameters> ComponentByFile::GetDynParameters() {
  reset_error();
  if (xml_doc_ == nullptr)
    return nullptr;
  // critical point parameters
  double cp[CP_PARAMETERS_COUNT];
  // heat capacities
  double hcv, hcp;
  std::vector<std::string> tmp_vec(
    dynamic_point_path);
  tmp_vec.push_back("");
  for (int i = 0; i < CP_PARAMETERS_COUNT; ++i)
    cp[i] = get_val(tmp_vec, cp_names[i], xml_doc_);
  // cp[CP_PRESSURE] *= 1000000;
  // fuuuuuuuuuuuuuu
  tmp_vec[XML_PATHLEN_SUBGROUP - 1] =
      dynamic_capacities_path[XML_PATHLEN_SUBGROUP - 1];
  hcv = get_val(tmp_vec, "heat_cap_vol", xml_doc_);
  hcp = get_val(tmp_vec, "heat_cap_pres", xml_doc_);
  return std::shared_ptr<dyn_parameters>(dyn_parameters::Init(
      hcv, hcp, get_intern_energy(hcv, cp[CP_TEMPERATURE]),
      parameters{cp[0], cp[1], cp[2]}));
}

void ComponentByFile::set_gas_name() {
  auto x = gas_names.find(xml_doc_->GetRootName());
  if (x != gas_names.end())
    gas_name_ = x->second;
}

ComponentByFile::~ComponentByFile() {
  if (xml_doc_ != nullptr)
    delete xml_doc_;
}
