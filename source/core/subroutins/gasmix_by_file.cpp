/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#include "gasmix_by_file.h"

#include "common.h"
#include "ErrorWrap.h"
#include "models_math.h"

#include <algorithm>
#include <map>

#include <assert.h>
#include <string.h>
#include <stdio.h>


namespace {
#if defined (_DEBUG_SUBROUTINS)
const char *gases_root_dir = "../../asp_therm/data/gases/";
#elif defined (_RELEASE)
const char *gases_root_dir = "data/gases/";
#endif  // _DEBUG_SUBROUTINS

std::string gasmix_component = "component";
std::string gasmix_parameter_name = "name";
std::string gasmix_parameter_path = "path";
std::string gasmix_parameter_part = "part";
}  // unnamed namespace

/** \brief Возвращает имя компонента газовой смеси по
  *   путь к файлу файлу газовой смеси и относительному пути указанному в нём
  * \param gasmix_file_dir директория расположения файла газовой смеси
  * \param gaspath путь к файлу параметров компонентов из файла описываюшего
  *   газовую смесь */
static std::string set_gaspath(const std::string &gasmix_file_dir,
    const std::string &gaspath);
/** \warning DEPRECATED(remove this)
  * \brief Возвращает имя компонента газовой смеси по
  * путь к дефолтному файлу параметров
  * \param path путь к дефолтному файлу параметров компонентов */
static std::string gasname_by_path(const std::string &path);

GasMixComponentsFile::GasMixComponentsFile(rg_model_t mn,
    XMLReader<gasmix_node> *xml_doc)
  : xml_doc_(xml_doc), files_handler_(nullptr),
    model_conf_(mn), error_(ERROR_SUCCESS_T) {
  init_components();
}

void GasMixComponentsFile::init_components() {
  std::string gasmix_file_dir = (xml_doc_) ?
      dir_by_path(xml_doc_->GetFileName()) : "";
  merror_t error = ERROR_SUCCESS_T;
  if (gasmix_file_dir.empty() || !is_exist(gasmix_file_dir)) {
    std::string error_str = "ошибка инициализации директории файла"
        "описывающего газовую смесь:\n  файл:" + xml_doc_->GetFileName() +
        "\n  директория:" + gasmix_file_dir;
    error_.SetError(ERROR_FILEIO_T | ERROR_FILE_IN_ST, error_str);
    error_.LogIt();
  } else {
    gasmix_files_.clear();
    char buf[64] = {0};
    std::string gasname;
    std::string gaspath;
    std::string part_str = "";
    double part = 0.0;
    std::vector<std::string> xml_path(2);
    for (int i = 0; i < GASMIX_MAX_COUNT; ++i) {
      gasname = "";
      part = 0.0;
      memset(buf, 0, sizeof(buf));
      sprintf(buf, "%s%d", gasmix_component.c_str(), i+1);
      xml_path[0] = std::string(buf);
      xml_path[1] = gasmix_parameter_name;
      error = xml_doc_->GetValueByPath(xml_path, &gasname);
      xml_path[1] = gasmix_parameter_path;
      error = xml_doc_->GetValueByPath(xml_path, &gaspath);
      xml_path[1] = gasmix_parameter_part;
      error |= xml_doc_->GetValueByPath(xml_path, &part_str);
      if (error)
        break;
      try {
        part = std::stod(part_str);
      } catch (std::out_of_range &) {
        error = error_.SetError(ERROR_STRING_T | ERROR_STR_PARSE_ST);
      }
      gasmix_files_.emplace_back(gasmix_file(trim_str(gasname), gaspath, part));
    }
    if (error == XML_LAST_STRING) {
      error = ERROR_SUCCESS_T;
      setup_gasmix_files(gasmix_file_dir);
    }
    if (error) {
      error_.SetError(error);
      error_.LogIt();
    }
  }
}

void GasMixComponentsFile::setup_gasmix_files(const std::string &gasmix_dir) {
  for (auto &x : gasmix_files_) {
    x.path = set_gaspath(gasmix_dir, x.path);
    x.part /= 100.0;
  }
  if (gasmix_files_.empty()) {
    error_.SetError(ERROR_FILEIO_T | ERROR_FILE_IN_ST,
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

// GasMix class
GasMixByFiles *GasMixByFiles::Init(const std::vector<gasmix_file> &parts) {
  if (check_input(parts))
    return nullptr;
  return new GasMixByFiles(parts);
}

GasMixByFiles::GasMixByFiles(const std::vector<gasmix_file> &parts)
  : prs_mix_(nullptr), error_(ERROR_SUCCESS_T), is_valid_(true) {
  prs_mix_ = std::unique_ptr<parameters_mix>(new parameters_mix());
  gost_mix_ = std::unique_ptr<ng_gost_mix>(new ng_gost_mix());
  for (const auto &x : parts) {
    auto cdp = init_pars(x.part, x.path);
    if (cdp.first == nullptr) {
      error_.SetError(ERROR_INIT_T,
          "One component of gas mix:\n  " + x.name);
      is_valid_ = false;
      break;
    }
    prs_mix_->insert({x.part, {*cdp.first, *cdp.second}});
  }
}

// finished
merror_t GasMixByFiles::check_input(const std::vector<gasmix_file> &parts) {
  merror_t err = ERROR_SUCCESS_T;
  if (parts.empty())
    GasParameters::init_error.SetError(ERROR_INIT_ZERO_ST, "init mix by file empty\n");
  // check parts > 0 and sum pf parts == 1.0
  double parts_sum = 0;
  for (const auto &x: parts) {
    if (x.part >= 0.0) {
      parts_sum += x.part;
    } else {
      GasParameters::init_error.SetError(ERROR_INIT_ZERO_ST, "init mix by files part <=0\n");
      break;
    }
  }
  if ((!err) && (!is_equal(parts_sum, 1.0, GASMIX_PERCENT_EPS)))
      GasParameters::init_error.SetError(
          ERROR_CALC_MIX_ST, "gasmix sum of part != 100%\n");
  return err;
}

std::pair<std::shared_ptr<const_parameters>, std::shared_ptr<dyn_parameters>>
    GasMixByFiles::init_pars(double part, const std::string &filename) {
  std::shared_ptr<ComponentByFile> xf(ComponentByFile::Init(filename));
  std::pair<std::shared_ptr<const_parameters>,
      std::shared_ptr<dyn_parameters>> ret_val{nullptr, nullptr};
  gas_t gost_mix_component;
  if (xf == nullptr) {
    error_.SetError(ERROR_FILEIO_T | ERROR_FILE_IN_ST, "gas xml init\n");
  } else {
    // unique_ptrs
    auto cp = xf->GetConstParameters();
    auto dp = xf->GetDynParameters();
    if ((cp == nullptr) || (dp == nullptr)) {
      error_.SetError(ERROR_INIT_T, "xml format and const/dyn -parameters\n");
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

static std::string set_gaspath(const std::string &gasmix_file_dir,
    const std::string &gaspath) {
  return gasmix_file_dir + "/" + trim_str(gaspath);
}

static std::string gasname_by_path(const std::string &path) {
  auto x = path.rfind(".xml");
  return (x != std::string::npos) ? std::string(path.begin() +
      path.rfind(PATH_SEPARATOR, x) + 1, path.begin() + x) : "";
}
