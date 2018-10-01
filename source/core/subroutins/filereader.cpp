#include "filereader.h"

#include "models_errors.h"
#include "models_math.h"

#include <algorithm>

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

// ===========================================================================
// implicit declarations
// ===========================================================================
double get_val(std::vector<std::string> &vec, const std::string &valname,
    XMLReader *xml_doc) {
  std::string tmp_str = "";
  vec[XML_PATHLEN_SUBGROUP] = valname;
  // код возврата можно не проверять, не в этом слyчае
  xml_doc->GetValueByPath(vec, &tmp_str);
  return std::stod(tmp_str);
}

// не оч точная функция
double get_intern_energy(double cv, double temper) {
  return cv * temper;
}
}  // unnamed namespace

bool operator< (const gas_mix_file &lg, const gas_mix_file &rg) {
  return strcmp(lg.filename.c_str(), rg.filename.c_str()) <= 0;
}

// ===========================================================================
// XmlGas class
// ===========================================================================
XmlFile::XmlFile(XMLReader *xml_doc)
  : xml_doc_(xml_doc) {}

XmlFile *XmlFile::Init(const std::string filename) {
  XMLReader *xml_doc = XMLReader::Init(filename);
  if (xml_doc == nullptr)
    return nullptr;
  return new XmlFile(xml_doc);
}

std::shared_ptr<const_parameters> XmlFile::GetConstParameters() {
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
  return std::shared_ptr<const_parameters>(const_parameters::Init(
      cp[0], cp[1], cp[2], mol, af));
}

std::shared_ptr<dyn_parameters> XmlFile::GetDynParameters() {
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

XmlFile::~XmlFile() {
  if (xml_doc_ != nullptr)
    delete xml_doc_;
}

// ===========================================================================
// GasMix class
// ===========================================================================
GasMix *GasMix::Init(const std::vector<gas_mix_file> &parts) {
  if (check_input(parts))
    return nullptr;
  return new GasMix(parts);
}

GasMix::GasMix(const std::vector<gas_mix_file> &parts)
  : is_valid_(true), prs_mix_(nullptr) {
  prs_mix_ = std::unique_ptr<parameters_mix>(new parameters_mix());
  for (const auto &x : parts) {
    auto cdp = init_pars(x.filename);
    if (cdp.first == nullptr) {
      set_error_message(ERR_INIT_T, "One component of gas mix: ");
      add_to_error_msg(x.filename.c_str());
      is_valid_ = false;
      break;
    }
    prs_mix_->insert({x.part, {*cdp.first, *cdp.second}});
  }
}

// finished
int GasMix::check_input(const std::vector<gas_mix_file> &parts) {
  if (parts.empty()) {
    set_error_message(ERR_INIT_T | ERR_INIT_ZERO_ST, "init mix by file empty");
    return ERR_INIT_T;
  }
  // check parts > 0 and sum pf parts == 1.0
  double parts_sum = 0;
  for (const auto &x: parts) {
    if (x.part >= 0.0) {
      parts_sum += x.part;
    } else {
      set_error_message(ERR_INIT_T | ERR_INIT_ZERO_ST,
          "init mix by files part <=0");
      return ERR_INIT_T;
    }
  }
  if (!is_equal(parts_sum, 1.0, GAS_MIX_PERCENT_EPS)) {
    set_error_message(ERR_CALCULATE_T | ERR_CALC_MIX_ST, "gas components");
    return ERR_INIT_T;
  }
  return ERR_SUCCESS_T;
}

std::pair<std::shared_ptr<const_parameters>, std::shared_ptr<dyn_parameters>>
    GasMix::init_pars(const std::string filename) {
  std::shared_ptr<XmlFile> xf(XmlFile::Init(filename));
  if (xf == nullptr) {
    set_error_message(ERR_FILEIO_T | ERR_FILE_IN_ST, "gas xml init");
    return {nullptr, nullptr};
  }
  // unique_ptrs
  auto cp = xf->GetConstParameters();
  auto dp = xf->GetDynParameters();
  if ((cp == nullptr) || (dp == nullptr)) {
    set_error_message(ERR_INIT_T, "xml format and const/dyn -parameters");
    /*
    if (cp != nullptr) {
      delete dp;
    } else {
      delete cp;
      if (dp != nullptr)
        delete dp;
    }
    */
    return {nullptr, nullptr};
  }
  return {cp, dp};
}

std::shared_ptr<parameters_mix> GasMix::GetParameters() {
  return (is_valid_) ? prs_mix_ : nullptr;
}
