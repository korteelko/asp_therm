/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020-2021 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#ifndef _CORE__SUBROUTINS__GASMIX_BY_FILE_H_
#define _CORE__SUBROUTINS__GASMIX_BY_FILE_H_

#include "asp_utils/ErrorWrap.h"
#include "asp_utils/FileURL.h"
#include "atherm_common.h"
#include "file_structs.h"
#include "gas_by_file.h"
#include "gas_description.h"
#include "gas_description_mix.h"
#include "models_math.h"
#if defined(WITH_PUGIXML)
#include "xml_reader.h"
#endif  // WITH_PUGIXML

#include <memory>
#include <string>
#include <type_traits>
#include <vector>

#include <assert.h>
#include <string.h>

/** \brief класс иницализации газовой смеси по переданным молярным долям
 *   и путям к файлам с газовыми параметрами(структурам 'gasmix_file') */
template <template <class gas_node> class ConfigReader>
class GasMixByFiles {
  GasMixByFiles(const GasMixByFiles&) = delete;
  GasMixByFiles& operator=(const GasMixByFiles&) = delete;

 public:
  static GasMixByFiles* Init(const std::vector<gasmix_component_info>& parts) {
    if (check_composition(parts))
      return nullptr;
    return new GasMixByFiles(parts);
  }

  std::shared_ptr<parameters_mix> GetMixParameters() {
    return (is_status_ok(status_) && !prs_mix_error_) ? prs_mix_ : nullptr;
  }

  std::shared_ptr<ng_gost_mix> GetGostMixParameters() {
    return (is_status_ok(status_) && !gost_mix_error_) ? gost_mix_ : nullptr;
  }

 public:
  static ErrorWrap init_error;

 private:
  /** \brief проверить состав газовой смеси */
  static merror_t check_composition(
      const std::vector<gasmix_component_info>& parts) {
    merror_t err = ERROR_SUCCESS_T;
    if (parts.empty())
      GasMixByFiles::init_error.SetError(ERROR_INIT_ZERO_ST,
                                         "init mix by file empty\n");
    // check parts > 0 and sum pf parts == 1.0
    double parts_sum = 0;
    for (const auto& x : parts) {
      if (x.part >= 0.0) {
        parts_sum += x.part;
      } else {
        GasMixByFiles::init_error.SetError(ERROR_INIT_ZERO_ST,
                                           "init mix by files part <=0\n");
        break;
      }
    }
    if ((!err) && (!is_equal(parts_sum, 1.0, GASMIX_PERCENT_EPS)))
      GasMixByFiles::init_error.SetError(ERROR_CALC_MIX_ST,
                                         "gasmix sum of part != 100%\n");
    return err;
  }

  GasMixByFiles(const std::vector<gasmix_component_info>& parts)
      : status_(STATUS_DEFAULT) {
    prs_mix_ = std::unique_ptr<parameters_mix>(new parameters_mix());
    prs_mix_error_ = ERROR_SUCCESS_T;
    gost_mix_ = std::unique_ptr<ng_gost_mix>(new ng_gost_mix());
    gost_mix_error_ = ERROR_SUCCESS_T;
    for (const auto& x : parts) {
      auto cdp = init_pars(x.part, x.name, x.path);
      if (!prs_mix_error_)
        prs_mix_->insert({x.part, {*cdp.first, *cdp.second}});
    }
    if ((!prs_mix_error_ || !gost_mix_error_) && is_status_aval(status_)) {
      status_ = STATUS_OK;
    }
  }

  std::pair<std::shared_ptr<const_parameters>, std::shared_ptr<dyn_parameters>>
  init_pars(double part, const std::string& name, const std::string& filename) {
    std::pair<std::shared_ptr<const_parameters>,
              std::shared_ptr<dyn_parameters>>
        ret_val{nullptr, nullptr};
    if (!prs_mix_error_) {
      std::shared_ptr<ComponentByFile<ConfigReader>> xf(
          ComponentByFile<ConfigReader>::Init(filename));
      if (xf == nullptr) {
        /* Если путь к файлу пуст, зарегистрируем ошибку для смеси gasmix  */
        // это ошибка инициализации обычного микса, для госта можно продолжить
        prs_mix_error_ |= error_.SetError(
            ERROR_FILE_IN_ST,
            "Ошибка инициализации файла компонента смеси: " + filename);
        Logging::Append(io_loglvl::warn_logs,
                        "Не найден файл конфигурации "
                        "для компонента "
                            + name);
      } else {
        // unique_ptrs
        auto cp = xf->GetConstParameters();
        auto dp = xf->GetDynParameters();
        if ((cp == nullptr) || (dp == nullptr)) {
          error_.SetError(ERROR_INIT_T,
                          "Ошибка инициализации const/dyn "
                          "параметров смеси для "
                              + filename);
        } else {
          ret_val = {cp, dp};
        }
      }
    }
    if (!gost_mix_error_) {
      gas_t gost_mix_component = gas_by_name(name);
      if (gost_mix_component != GAS_TYPE_UNDEFINED) {
        gost_mix_->emplace_back(ng_gost_component{gost_mix_component, part});
      } else {
        // ошибка инициализации гост-смеси
        gost_mix_error_ |= ERROR_INIT_T;
        Logging::Append(io_loglvl::warn_logs,
                        "Неизвестный компонент смеси " + name);
      }
    }
    return ret_val;
  }

 private:
  ErrorWrap error_;
  mstatus_t status_;
  std::shared_ptr<parameters_mix> prs_mix_;
  merror_t prs_mix_error_;
  std::shared_ptr<ng_gost_mix> gost_mix_;
  merror_t gost_mix_error_;
};

template <template <class gas_node> class ConfigReader>
ErrorWrap GasMixByFiles<ConfigReader>::init_error;

/* TODO: redo this(hide) */
namespace {
std::string gasmix_component = "component";
std::string gasmix_parameter_name = "name";
std::string gasmix_parameter_path = "path";
std::string gasmix_parameter_part = "part";
}  // unnamed namespace

/**
 * \brief Шаблон(по модулю чтения) класса инициализации файловой смеси
 * \note Привязан шаблон к 'gas_node' нерушимой связью
 *   и можно псевдоним ввести для этого
 * */
template <template <class gas_node> class ConfigReader>
class GasMixComponentsFile {
  GasMixComponentsFile(const GasMixComponentsFile&) = delete;
  GasMixComponentsFile& operator=(const GasMixComponentsFile&) = delete;

 public:
  /**
   * \brief Инициализировать параметры газовой смеси
   * \param mn Идентификатор модели
   * \param root Указатель на корневую директорию компонентов
   *   газовой смеси(может быть равен нулю)
   * \param filename Путь к файлу конфигурации газовой смеси
   * \return Указатель на модель
   * */
  static GasMixComponentsFile* Init(rg_model_t mn,
                                    file_utils::FileURLRoot* root,
                                    const std::string& filename) {
    ConfigReader<gasmix_node>* config_doc =
        ConfigReader<gasmix_node>::Init(filename);
    if (config_doc == nullptr)
      return nullptr;
    return new GasMixComponentsFile(mn, root, config_doc);
  }

  std::shared_ptr<parameters_mix> GetMixParameters() {
    std::shared_ptr<parameters_mix> params_p = nullptr;
    if (model_t_ != rg_model_t::NG_GOST)
      if (files_handler_ != nullptr)
        params_p = files_handler_->GetMixParameters();
    return params_p;
  }

  std::shared_ptr<ng_gost_mix> GetGostMixParameters() {
    return (files_handler_ != nullptr) ? files_handler_->GetGostMixParameters()
                                       : nullptr;
  }

 private:
  GasMixComponentsFile(rg_model_t mn,
                       file_utils::FileURLRoot* root,
                       ConfigReader<gasmix_node>* config_doc)
      : model_t_(mn), config_doc_(config_doc) {
    init_components(root);
  }
  /**
   * \brief Инициализировать компоненты смеси
   * \param root Указатель на корневую директорию компонентов
   *   газовой смеси(может быть равен нулю)
   * */
  void init_components(file_utils::FileURLRoot* root) {
    std::string gasmix_file_dir = "";
    if (root != nullptr) {
      gasmix_file_dir = root->GetRootURL().GetURL();
    } else {
      if (config_doc_ != nullptr)
        gasmix_file_dir = dir_by_path(config_doc_->GetFileName());
    }

    merror_t error = ERROR_SUCCESS_T;
    if (gasmix_file_dir.empty() || !is_exists(gasmix_file_dir)) {
      std::string error_str =
          "ошибка инициализации директории файла"
          "описывающего газовую смесь:\n  файл:"
          + config_doc_->GetFileName() + "\n  директория:" + gasmix_file_dir;
      error_.SetError(ERROR_FILE_IN_ST, error_str);
      error_.LogIt();
    } else {
      gasmix_files_.clear();
      char buf[64] = {0};
      std::string gasname;
      std::string gpath;
      std::string part_str = "";
      double part = 0.0;
      std::vector<std::string> param_path(2);
      for (int i = 0; i < GASMIX_MAX_COUNT; ++i) {
        gasname = "";
        part = 0.0;
        memset(buf, 0, sizeof(buf));
        // todo: решение просто 10/10
        snprintf(buf, sizeof(buf) - 1, "%s%d", gasmix_component.c_str(), i + 1);
        param_path[0] = std::string(buf);
        param_path[1] = gasmix_parameter_name;
        error = config_doc_->GetValueByPath(param_path, &gasname);
        param_path[1] = gasmix_parameter_path;
        error = config_doc_->GetValueByPath(param_path, &gpath);
        param_path[1] = gasmix_parameter_part;
        error |= config_doc_->GetValueByPath(param_path, &part_str);
        if (error)
          break;
        try {
          part = std::stod(part_str);
        } catch (std::out_of_range&) {
          error = error_.SetError(ERROR_PAIR_DEFAULT(ERROR_STR_PARSE_ST));
        }
        gasmix_files_.emplace_back(
            gasmix_component_info(trim_str(gasname), trim_str(gpath), part));
      }
      if (error == XMLFILE_LAST_OBJECT) {
        error = ERROR_SUCCESS_T;
        setup_gasmix_files(gasmix_file_dir);
      }
      if (error) {
        error_.SetError(error, "ошибка инициализации компонентов смеси");
        error_.LogIt();
      }
    }
  }

  void setup_gasmix_files(const std::string& gasmix_dir) {
    for (auto& x : gasmix_files_) {
      x.path = set_gaspath(gasmix_dir, x.path);
      x.part /= 100.0;
    }
    if (gasmix_files_.empty()) {
      error_.SetError(ERROR_FILE_IN_ST,
                      "gasmix input file is empty or broken\n");
    } else {
      files_handler_ = std::unique_ptr<GasMixByFiles<ConfigReader>>(
          GasMixByFiles<ConfigReader>::Init(gasmix_files_));
    }
  }
  /**
   * \brief Возвращает имя компонента газовой смеси по
   *   путь к файлу файлу газовой смеси и относительному пути указанному в нём
   * \param gasmix_file_dir директория расположения файла газовой смеси
   * \param gaspath путь к файлу параметров компонентов из файла описываюшего
   *   газовую смесь
   * */
  std::string set_gaspath(const std::string& gasmix_file_dir,
                          const std::string& gaspath) {
    return gasmix_file_dir + "/" + trim_str(gaspath);
  }
  // void setup_gost_mix();

 private:
  ErrorWrap error_;
  /**
   * \brief Используемая термодинамическая модель
   * \note Она не нужна, нужна просто bool переменная на
   *   считывание файлов, или вообще не нужна, т.к. в файлах миксов
   *   прописаны пути к компонентам их описывающим. Соответственно,
   *   если путь не задан то не о чём и говорить
   * \todo Remove it. You can replace with `bool should_read_components_`)
   * */
  rg_model_t model_t_;
  /**
   * \brief Указатель на объект инициализации компонента смеси
   * */
  std::unique_ptr<ConfigReader<gasmix_node>> config_doc_ = nullptr;
  /**
   * \brief Объект инициализирующий смесь
   * \todo Имя его неподходящее
   * */
  std::unique_ptr<GasMixByFiles<ConfigReader>> files_handler_ = nullptr;
  /**
   * \brief Контейнер считанных данных
   * */
  std::vector<gasmix_component_info> gasmix_files_;
};

#endif  // !_CORE__SUBROUTINS__GASMIX_BY_FILE_H_
