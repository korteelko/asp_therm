/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#ifndef _CORE__SUBROUTINS__GASMIX_BY_FILE_H_
#define _CORE__SUBROUTINS__GASMIX_BY_FILE_H_

#include "common.h"
#include "file_structs.h"
#include "gas_by_file.h"
#include "gas_description.h"
#include "gasmix_init.h"
#include "models_math.h"
#include "ErrorWrap.h"
#if defined (WITH_RAPIDJSON)
#  include "JSONReader.h"
#endif  // WITH_RAPIDJSON
#if defined (WITH_PUGIXML)
#  include "XMLReader.h"
#endif  // WITH_PUGIXML

#include <memory>
#include <string>
#include <type_traits>
#include <vector>

#include <assert.h>
#include <string.h>


/** \brief класс иницализации газовой смеси по переданным молярным долям
  *   и путям к файлам с газовыми параметрами(структурам 'gasmix_file') */
template <template<class gas_node> class ConfigReader>
class GasMixByFiles {
  GasMixByFiles(const GasMixByFiles &) = delete;
  GasMixByFiles &operator=(const GasMixByFiles &) = delete;

public:
  static GasMixByFiles *Init(const std::vector<gasmix_file> &parts) {
    if (check_composition(parts))
      return nullptr;
    return new GasMixByFiles(parts);
  }

  std::shared_ptr<parameters_mix> GetMixParameters() {
    return (is_status_ok(status_)) ? prs_mix_ : nullptr;
  }

  std::shared_ptr<ng_gost_mix> GetGostMixParameters() {
    return gost_mix_;
  }

public:
  static ErrorWrap init_error;

private:
  /** \brief проверить состав газовой смеси */
  static merror_t check_composition(const std::vector<gasmix_file> &parts) {
    merror_t err = ERROR_SUCCESS_T;
    if (parts.empty())
      GasMixByFiles::init_error.SetError(
          ERROR_INIT_ZERO_ST, "init mix by file empty\n");
    // check parts > 0 and sum pf parts == 1.0
    double parts_sum = 0;
    for (const auto &x: parts) {
      if (x.part >= 0.0) {
        parts_sum += x.part;
      } else {
        GasMixByFiles::init_error.SetError(
            ERROR_INIT_ZERO_ST, "init mix by files part <=0\n");
        break;
      }
    }
    if ((!err) && (!is_equal(parts_sum, 1.0, GASMIX_PERCENT_EPS)))
      GasMixByFiles::init_error.SetError(
           ERROR_CALC_MIX_ST, "gasmix sum of part != 100%\n");
    return err;
  }

  GasMixByFiles(const std::vector<gasmix_file> &parts)
    : status_(STATUS_DEFAULT) {
    prs_mix_ = std::unique_ptr<parameters_mix>(new parameters_mix());
    gost_mix_ = std::unique_ptr<ng_gost_mix>(new ng_gost_mix());
    for (const auto &x : parts) {
      auto cdp = init_pars(x.part, x.path);
      if (cdp.first == nullptr) {
        error_.SetError(ERROR_INIT_T,
            "One component of gas mix:\n  " + x.name);
        error_.LogIt();
        status_ = STATUS_HAVE_ERROR;
        break;
      }
      prs_mix_->insert({x.part, {*cdp.first, *cdp.second}});
    }
    if (!error_.GetErrorCode())
      status_ = STATUS_OK;
  }

  std::pair<std::shared_ptr<const_parameters>, std::shared_ptr<dyn_parameters>>
      init_pars(double part, const std::string &filename) {
    std::shared_ptr<ComponentByFile<ConfigReader>>
        xf(ComponentByFile<ConfigReader>::Init(filename));
    std::pair<std::shared_ptr<const_parameters>,
        std::shared_ptr<dyn_parameters>> ret_val{nullptr, nullptr};
    gas_t gost_mix_component;
    if (xf == nullptr) {
      error_.SetError(ERROR_FILEIO_T | ERROR_FILE_IN_ST, "gas config init\n");
    } else {
      // unique_ptrs
      auto cp = xf->GetConstParameters();
      auto dp = xf->GetDynParameters();
      if ((cp == nullptr) || (dp == nullptr)) {
        error_.SetError(ERROR_INIT_T, "config format and const/dyn -parameters\n");
      } else {
        ret_val = {cp, dp};
      }
    }
    gost_mix_component = gas_by_name(gasname_by_path(filename));
    if (gost_mix_component != GAS_TYPE_UNDEFINED)
     gost_mix_->emplace_back(ng_gost_component{gost_mix_component, part});
    return ret_val;
  }

/** \warning DEPRECATED(remove this)
  * \brief Возвращает имя компонента газовой смеси по
  * путь к дефолтному файлу параметров
  * \param path путь к дефолтному файлу параметров компонентов */
  static std::string gasname_by_path(const std::string &path) {
    std::string reader_ext = ConfigReader<gas_node>::GetFilenameExtension();
    auto x = path.rfind(reader_ext);
    return (x != std::string::npos) ? std::string(path.begin() +
        path.rfind(PATH_SEPARATOR, x) + 1, path.begin() + x) : "";
  }

private:
  ErrorWrap error_;
  mstatus_t status_;
  std::shared_ptr<parameters_mix> prs_mix_;
  std::shared_ptr<ng_gost_mix> gost_mix_;
};

template <template<class gas_node> class ConfigReader>
ErrorWrap GasMixByFiles<ConfigReader>::init_error;


/* todo: redo this(hide) */
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


/** \brief шаблон(по модулю чтения) класса инициализации файловой смеси
  * \note привязан шаблон к 'gas_node' нерушимой связью
  *   и можно псевдоним ввести для этого */
template <template<class gas_node> class ConfigReader>
class GasMixComponentsFile {
  GasMixComponentsFile(const GasMixComponentsFile &) = delete;
  GasMixComponentsFile &operator=(const GasMixComponentsFile &) = delete;

public:
  static GasMixComponentsFile *Init(rg_model_t mn,
      const std::string &filename) {
    ConfigReader<gasmix_node> *config_doc =
        ConfigReader<gasmix_node>::Init(filename);
    if (config_doc == nullptr)
      return nullptr;
    return new GasMixComponentsFile(mn, config_doc);
  }

  std::shared_ptr<parameters_mix> GetMixParameters() {
    std::shared_ptr<parameters_mix> params_p = nullptr;
    if (model_t_ != rg_model_t::NG_GOST)
      if (files_handler_ != nullptr)
        params_p = files_handler_->GetMixParameters();
    return params_p;
  }

  std::shared_ptr<ng_gost_mix> GetGostMixParameters() {
    return (files_handler_ != nullptr) ?
        files_handler_->GetGostMixParameters() : nullptr;
  }

private:
  GasMixComponentsFile(rg_model_t mn, ConfigReader<gasmix_node> *config_doc)
    : config_doc_(config_doc), files_handler_(nullptr),
      model_t_(mn), error_(ERROR_SUCCESS_T) {
    init_components();
  }
  /** \brief */
  void init_components() {
    std::string gasmix_file_dir = (config_doc_) ?
        dir_by_path(config_doc_->GetFileName()) : "";
    merror_t error = ERROR_SUCCESS_T;
    if (gasmix_file_dir.empty() || !is_exist(gasmix_file_dir)) {
      std::string error_str = "ошибка инициализации директории файла"
          "описывающего газовую смесь:\n  файл:" + config_doc_->GetFileName() +
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
      std::vector<std::string> param_path(2);
      for (int i = 0; i < GASMIX_MAX_COUNT; ++i) {
        gasname = "";
        part = 0.0;
        memset(buf, 0, sizeof(buf));
        sprintf(buf, "%s%d", gasmix_component.c_str(), i+1);
        param_path[0] = std::string(buf);
        param_path[1] = gasmix_parameter_name;
        error = config_doc_->GetValueByPath(param_path, &gasname);
        param_path[1] = gasmix_parameter_path;
        error = config_doc_->GetValueByPath(param_path, &gaspath);
        param_path[1] = gasmix_parameter_part;
        error |= config_doc_->GetValueByPath(param_path, &part_str);
        if (error)
          break;
        try {
          part = std::stod(part_str);
        } catch (std::out_of_range &) {
          error = error_.SetError(ERROR_STRING_T | ERROR_STR_PARSE_ST);
        }
        gasmix_files_.emplace_back(gasmix_file(trim_str(gasname), gaspath, part));
      }
      if (error == FILE_LAST_OBJECT) {
        error = ERROR_SUCCESS_T;
        setup_gasmix_files(gasmix_file_dir);
      }
      if (error) {
        error_.SetError(error);
        error_.LogIt();
      }
    }
  }

  void setup_gasmix_files(const std::string &gasmix_dir) {
    for (auto &x : gasmix_files_) {
      x.path = set_gaspath(gasmix_dir, x.path);
      x.part /= 100.0;
    }
    if (gasmix_files_.empty()) {
      error_.SetError(ERROR_FILEIO_T | ERROR_FILE_IN_ST,
          "gasmix input file is empty or broken\n");
    } else {
      files_handler_ = std::unique_ptr<GasMixByFiles<ConfigReader>>(
          GasMixByFiles<ConfigReader>::Init(gasmix_files_));
    }
  }
  /** \brief Возвращает имя компонента газовой смеси по
    *   путь к файлу файлу газовой смеси и относительному пути указанному в нём
    * \param gasmix_file_dir директория расположения файла газовой смеси
    * \param gaspath путь к файлу параметров компонентов из файла описываюшего
    *   газовую смесь */
  std::string set_gaspath(const std::string &gasmix_file_dir,
      const std::string &gaspath) {
    return gasmix_file_dir + "/" + trim_str(gaspath);
  }
  // void setup_gost_mix();

private:
  std::unique_ptr<ConfigReader<gasmix_node>> config_doc_;
  /* maybe remove files_handler_ */
  std::unique_ptr<GasMixByFiles<ConfigReader>> files_handler_;
  /* containers
   * UPD: lol, we can recalculate gasmix_files_ by gost model */
  std::vector<gasmix_file> gasmix_files_;
  rg_model_t model_t_;
  ErrorWrap error_;
};

#endif  // !_CORE__SUBROUTINS__GASMIX_BY_FILE_H_
