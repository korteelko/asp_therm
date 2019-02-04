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
const int constint_64 = 64;
#ifdef _DEBUG_SUBROUTINS
const char *gases_root_dir = "../../asp_therm/data/gases/";
#else
const char *gases_root_dir = "../../data/gases/";
#endif  // _DEBUG_SUBROUTINS

std::string gasmix_component = "component";
std::string gasmix_parameter_name = "name";
std::string gasmix_parameter_part = "part";
}  // unnamed namespace

GasMixComponentsFile::GasMixComponentsFile(XMLReader<gasmix_node> *xml_doc) 
  : files_handler_(nullptr), xml_doc_(xml_doc) {
  init_components();
}

void GasMixComponentsFile::init_components() {
  gasmix_files_.clear();
  char buf[constint_64] = {0};
  std::string gasname;
  std::string part_str="";
  int part = 0.0;
  std::vector<std::string> xml_path(2);
  for (int i = 0; i < 32; ++i) {
    gasname = "";
    part = 0.0;
    memset(buf, 0, constint_64);
    sprintf(buf, "%s%d", gasmix_component.c_str(), i+1);
    xml_path[0] = std::string(buf);
    xml_path[1] = gasmix_parameter_name;
    error_t err = xml_doc_->GetValueByPath(xml_path, &gasname);
    if (err)
      break;
    xml_path[1] = gasmix_parameter_part;
    err = xml_doc_->GetValueByPath(xml_path, &part_str);
    part = std::stod(part_str);
    gasmix_files_.push_back(gasmix_file(gasname, part));
  }
  for (auto &x : gasmix_files_) {
    x.filename = gases_root_dir + trim_str(x.filename) + ".xml";
    x.part /= 100.0;
  }
  if (gasmix_files_.empty()) {
    set_error_message(ERR_FILEIO_T | ERR_FILE_IN_ST,
        "gasmix input file is empty or broken");
    return;
  }
  files_handler_ = std::unique_ptr<GasMixByFiles>(
      GasMixByFiles::Init(gasmix_files_));
  // и т.д.
  // update path
}

GasMixComponentsFile *GasMixComponentsFile::Init(const std::string &filename) {
  XMLReader<gasmix_node> *xml_doc = XMLReader<gasmix_node>::Init(filename);
  if (xml_doc == nullptr)
    return nullptr;
  return new GasMixComponentsFile(xml_doc);
}

std::shared_ptr<parameters_mix> GasMixComponentsFile::GetParameters() {
  if (files_handler_ == nullptr)
    return nullptr;
  return files_handler_->GetParameters();
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
int GasMixByFiles::check_input(const std::vector<gasmix_file> &parts) {
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
  if (!is_equal(parts_sum, 1.0, GASMIX_PERCENT_EPS)) {
    set_error_message(ERR_CALCULATE_T | ERR_CALC_MIX_ST, "gasmix sum of part != 100%");
    return ERR_INIT_T;
  }
  return ERR_SUCCESS_T;
}

std::pair<std::shared_ptr<const_parameters>, std::shared_ptr<dyn_parameters>>
    GasMixByFiles::init_pars(const std::string &filename) {
  std::shared_ptr<ComponentByFile> xf(ComponentByFile::Init(filename));
  if (xf == nullptr) {
    set_error_message(ERR_FILEIO_T | ERR_FILE_IN_ST, "gas xml init");
    return {nullptr, nullptr};
  }
  // unique_ptrs
  auto cp = xf->GetConstParameters();
  auto dp = xf->GetDynParameters();
  if ((cp == nullptr) || (dp == nullptr)) {
    set_error_message(ERR_INIT_T, "xml format and const/dyn -parameters");
    return {nullptr, nullptr};
  }
  return {cp, dp};
}

std::shared_ptr<parameters_mix> GasMixByFiles::GetParameters() {
  return (is_valid_) ? prs_mix_ : nullptr;
}
