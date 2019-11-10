#include "gasmix_by_file.h"

#include "models_common.h"
#include "models_errors.h"
#include "models_math.h"

#include <algorithm>
#include <map>

#include <assert.h>
#include <string.h>
#include <stdio.h>

/*
 * !!! file gasmix example
 * !!!   input by file
 * <?xml version="1.0" encoding="UTF-8"?>
 * <gasmix name="example">
 *   <group name="component1">
 *     <parameter name="name"> methane </parameter>
 *     <parameter name="part"> 93.5 </parameter>
 *   </group>
 *   <group name="component2">
 *     <parameter name="name"> ethane </parameter>
 *     <parameter name="part"> 4.4 </parameter>
 *   </group>
 *   <group name="component3">
 *     <parameter name="name"> propane </parameter>
 *     <parameter name="part"> 2.1 </parameter>
 *   </group>
 * </gasmix> 
 */

namespace {
const int32_t constint_64 = 64;
#if defined (_DEBUG_SUBROUTINS)
const char *gases_root_dir = "../../asp_therm/data/gases/";
#elif defined (_RELEASE)
const char *gases_root_dir = "data/gases/";
#else
const char *gases_root_dir = "../data/gases/";
#endif  // _DEBUG_SUBROUTINS

std::string gasmix_component = "component";
std::string gasmix_parameter_name = "name";
std::string gasmix_parameter_part = "part";
}  // unnamed namespace

static std::string path_by_gasname(const std::string &gasname);
static std::string gasname_by_path(const std::string &path);

GasMixComponentsFile::GasMixComponentsFile(rg_model_t mn,
    XMLReader<gasmix_node> *xml_doc)
  : xml_doc_(xml_doc), files_handler_(nullptr),
    model_conf_(mn), error_(ERR_SUCCESS_T) {
  init_components();
}

void GasMixComponentsFile::init_components() {
  gasmix_files_.clear();
  char buf[constint_64] = {0};
  std::string gasname;
  std::string part_str = "";
  double part = 0.0;
  std::vector<std::string> xml_path(2);
  for (int i = 0; i < GASMIX_MAX_COUNT; ++i) {
    gasname = "";
    part = 0.0;
    memset(buf, 0, constint_64);
    sprintf(buf, "%s%d", gasmix_component.c_str(), i+1);
    xml_path[0] = std::string(buf);
    xml_path[1] = gasmix_parameter_name;
    error_ = xml_doc_->GetValueByPath(xml_path, &gasname);
    xml_path[1] = gasmix_parameter_part;
    error_ |= xml_doc_->GetValueByPath(xml_path, &part_str);
    if (error_)
      break;
    part = std::stod(part_str);
    gasmix_files_.emplace_back(gasmix_file(gasname, part));
  }
  error_ = ERR_SUCCESS_T;
  setup_gasmix_files();
}

void GasMixComponentsFile::setup_gasmix_files() {
  for (auto &x : gasmix_files_) {
    x.filename = path_by_gasname(x.filename);
    x.part /= 100.0;
  }
  if (gasmix_files_.empty()) {
    error_ = set_error_message(ERR_FILEIO_T | ERR_FILE_IN_ST,
        "gasmix input file is empty or broken\n");
  } else {
    files_handler_ = std::unique_ptr<GasMixByFiles>(
        GasMixByFiles::Init(gasmix_files_));
  }
}


GasMixComponentsFile *GasMixComponentsFile::Init(
    rg_model_t mn, const std::string &filename) {
  XMLReader<gasmix_node> *xml_doc = XMLReader<gasmix_node>::Init(filename);
  if (xml_doc == nullptr)
    return nullptr;
  return new GasMixComponentsFile(mn, xml_doc);
}

std::shared_ptr<parameters_mix> GasMixComponentsFile::GetMixParameters() {
  std::shared_ptr<parameters_mix> params_p = nullptr;
  if (model_conf_ != rg_model_t::NG_GOST)
    if (files_handler_ != nullptr)
      params_p = files_handler_->GetMixParameters();
  return params_p;
}
 
std::shared_ptr<ng_gost_mix> GasMixComponentsFile::GetGostMixParameters() {
  return (files_handler_ != nullptr) ?
      files_handler_->GetGostMixParameters() : nullptr;
}

GasMixComponentsFile::~GasMixComponentsFile() {
  if (xml_doc_ != nullptr)
    delete xml_doc_;
}
 
// GasMix class
GasMixByFiles *GasMixByFiles::Init(const std::vector<gasmix_file> &parts) {
  if (check_input(parts))
    return nullptr;
  return new GasMixByFiles(parts);
}

GasMixByFiles::GasMixByFiles(const std::vector<gasmix_file> &parts)
  : prs_mix_(nullptr), error_(ERR_SUCCESS_T), is_valid_(true) {
  prs_mix_ = std::unique_ptr<parameters_mix>(new parameters_mix());
  gost_mix_ = std::unique_ptr<ng_gost_mix>(new ng_gost_mix());
  for (const auto &x : parts) {
    auto cdp = init_pars(x.part, x.filename);
    if (cdp.first == nullptr) {
      error_ = set_error_message(ERR_INIT_T, "One component of gas mix:\n");
      add_to_error_msg(x.filename.c_str());
      is_valid_ = false;
      break;
    }
    prs_mix_->insert({x.part, {*cdp.first, *cdp.second}});
  }
  setup_gost_mix();
}

// finished
merror_t GasMixByFiles::check_input(const std::vector<gasmix_file> &parts) {
  merror_t err = ERR_SUCCESS_T;
  if (parts.empty())
    err = set_error_message(
        ERR_INIT_T | ERR_INIT_ZERO_ST, "init mix by file empty\n");
  // check parts > 0 and sum pf parts == 1.0
  double parts_sum = 0;
  for (const auto &x: parts) {
    if (x.part >= 0.0) {
      parts_sum += x.part;
    } else {
      err = set_error_message(
          ERR_INIT_T | ERR_INIT_ZERO_ST, "init mix by files part <=0\n");
      break;
    }
  }
  if ((!err) && (!is_equal(parts_sum, 1.0, GASMIX_PERCENT_EPS)))
    err = set_error_message(
        ERR_CALCULATE_T | ERR_CALC_MIX_ST, "gasmix sum of part != 100%\n");
  return err;
}

void GasMixByFiles::setup_gost_mix() {
  for (auto &x : *gost_mix_)
    x.second /= 100.0;
  if (gost_mix_->empty())
    error_ = set_error_message(ERR_FILEIO_T | ERR_FILE_IN_ST,
        "gasmix input file is empty or broken\n");
}

std::pair<std::shared_ptr<const_parameters>, std::shared_ptr<dyn_parameters>>
    GasMixByFiles::init_pars(double part, const std::string &filename) {
  std::shared_ptr<ComponentByFile> xf(ComponentByFile::Init(filename));
  std::pair<std::shared_ptr<const_parameters>,
      std::shared_ptr<dyn_parameters>> ret_val{nullptr, nullptr};
  gas_t gost_mix_component;
  if (xf == nullptr) {
    error_ = set_error_message(ERR_FILEIO_T | ERR_FILE_IN_ST, "gas xml init\n");
  } else {
    // unique_ptrs
    auto cp = xf->GetConstParameters();
    auto dp = xf->GetDynParameters();
    if ((cp == nullptr) || (dp == nullptr)) {
      error_ = set_error_message(
          ERR_INIT_T, "xml format and const/dyn -parameters\n");
    } else {
      ret_val = {cp, dp};
    }
  }
  gost_mix_component = gas_by_name(gasname_by_path(filename));
  if (gost_mix_component != GAS_TYPE_UNDEFINED)
    gost_mix_->emplace_back(ng_gost_component{gost_mix_component, part});
  return ret_val;
}

std::shared_ptr<parameters_mix> GasMixByFiles::GetMixParameters() {
  return (is_valid_) ? prs_mix_ : nullptr;
}

std::shared_ptr<ng_gost_mix> GasMixByFiles::GetGostMixParameters() {
  return gost_mix_;
}

/*
 * static
 */

static std::string path_by_gasname(const std::string &gasname) {
  return gases_root_dir + trim_str(gasname) + ".xml";
}

static std::string gasname_by_path(const std::string &path) {
  return std::string(path.c_str() + strlen(gases_root_dir));
}
