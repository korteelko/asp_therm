/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#include "calculation_setup.h"

#include "calculation_by_file.h"
#include "models_configurations.h"
#include "models_creator.h"

#include <algorithm>
#include <future>


calculation_setup::calculation_setup(
    std::shared_ptr<file_utils::FileURLRoot> &root)
  : root(root) {}

/* CalculationSetup */
CalculationSetup::CalculationSetup(std::shared_ptr<file_utils::FileURLRoot> &root,
    const std::string &filepath) {
  init_data_.reset(new calculation_setup(root));
  if (init_data_ != nullptr) {
    // todo: почти неиспользуемая переменная path
    auto path = root->CreateFileURL(filepath);
    initSetup(&path);
  } else {
    if (root != nullptr) {
      error_.SetError(ERROR_FILE_IN_ST, "ошибка инициализации файла"
          " конфигурации расчёта:\n\troot: " + root->CreateFileURL("").GetURL() +
          " \tфайл: " + filepath);
    } else {
      error_.SetError(ERROR_FILE_IN_ST, "ошибка инициализации файла"
        " конфигурации расчёта:\n\troot директория не задана " );
    }
  }
}

#if !defined(DATABASE_TEST)
/*
mstatus_t CalculationSetup::CheckCurrentModel() {
  if (current_model_ && is_status_ok(status_)) {
    // todo прописать этот свап
    // если использование выбранной модели не допустимо
    //   переключиться на другую
    if (!current_model_->IsValid()) {
      params_copy_ = current_model_->GetParametersCopy();
      swapModel();
    }
  }
  return status_;
}
*/
#endif  // !DATABASE_TEST

/*
merror_t CalculationSetup::SetModel(int model_key) {
  merror_t error = ERROR_GENERAL_T;
  const auto it = models_.find(model_key);
  if (it != models_.end()) {
    current_model_ = it->second.get();
    error = ERROR_SUCCESS_T;
  }
  return error;
}
*/

merror_t CalculationSetup::GetError() const {
  return error_.GetErrorCode();
}

merror_t CalculationSetup::initSetup(file_utils::FileURL *filepath_p) {
  // Прячем указатель на calculation_setup
  //   в класс-строитель
  CalculationSetupBuilder builder(init_data_.get());
  // Инициализируем ридер по адресу пути к файлу и классу строителю выше
  std::unique_ptr<ReaderSample<pugi::xml_node, calculation_node<pugi::xml_node>,
      CalculationSetupBuilder>> rs(ReaderSample<pugi::xml_node,
      calculation_node<pugi::xml_node>, CalculationSetupBuilder>
      ::Init(filepath_p, &builder));
  merror_t error = ERROR_SUCCESS_T;
  if (rs) {
    rs->InitData();
    if (!rs->GetErrorCode()) {
      // инициализировать расчётные модели
      if (initData()) {
        error = error_.SetError(ERROR_INIT_T, "ошибка иницианилизации"
            " структуры конфигурации расчёта");
      }
    } else {
      rs->LogError();
      error = error_.SetError(ERROR_READER_PARSE_ST,
          "ошибка парсинга файла конфигурации расчёта");
    }
  } else {
    error = error_.SetError(ERROR_READER_T,
        "ошибка чтения файла конфигурации расчёта");
  }
  return error;
}

merror_t CalculationSetup::initData() {
  // инициализация
  std::lock_guard lg(gasmixes_lock_);
  gasmix_models_map models_map;
  merror_t error = ERROR_SUCCESS_T;
  for (auto file: init_data_->gasmix_files) {
    auto path_str = init_data_->root->CreateFileURL(file).GetURL();
    std::vector<std::pair<std::future<std::shared_ptr<modelGeneral>>,
        rg_model_id *>> future_models;
    // todo: is ModelsCreator::GetCalculatingModel throw safe???
    auto creator_f = [] (const model_str &ms, const std::string &filemix) {
        return std::shared_ptr<modelGeneral>(
            ModelsCreator::GetCalculatingModel(ms, filemix)); };
    for (auto m: init_data_->models)
      future_models.push_back({ std::async(creator_f,
          modelGeneral::GetModelShortInfo(m), path_str), &m });
    for (auto &fm: future_models) {
      // обрабатываем ошибки создания модели
      auto m_ptr = fm.first.get();
      if (m_ptr) {
        models_map.models.emplace(m_ptr->GetPriority(), m_ptr);
      } else {
        Logging::Append(error = ERROR_INIT_NULLP_ST,
            "ошибка создания расчётной модели для файла: " + path_str +
            "\n\t" + modelGeneral::GetModelShortInfo(*fm.second).GetString());
      }
    }
  }
  // "скопировать" расчётные точки
  points_ = std::vector<parameters>(std::move(init_data_->points));
  // удаляем данные сетапа
  init_data_ = nullptr;
  return error;
}

/*
void CalculationSetup::swapModel() {
  status_ = STATUS_NOT;
  auto const model_it = models_.cbegin();
  while (model_it != models_.cend()) {
    // первая модель для которой допустимы макропараметры
    if (model_it->second->IsValid(params_copy_)) {
      status_ = STATUS_OK;
    }
  }
}
*/
