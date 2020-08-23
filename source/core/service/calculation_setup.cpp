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

#include "atherm_db_tables.h"
#include "calculation_by_file.h"
#include "db_connection_manager.h"
#include "models_configurations.h"
#include "models_creator.h"
#include "program_state.h"

#include <algorithm>
#include <future>


// тестим проблемс с ДБ
Mutex db_test;
// тестим проблемс с ДБ
// Mutex calc_test;

calculation_setup::calculation_setup(
    std::shared_ptr<file_utils::FileURLRoot> &root)
  : root(root) {}

/* CalculationSetup */
void CalculationSetup::gasmix_models_map::CalculatePoints(
    const std::vector<parameters> &points) {
  // todo: udoli
  // std::lock_guard<Mutex> lock(calc_test);
  initInfoBinding();
  if (is_status_aval(status))
    for (auto &p: points)
      calculatePoint(p);
}

mstatus_t CalculationSetup::gasmix_models_map::AddToDatabase(
    DBConnectionManager *source_ptr) {
  // todo: валит в exception
  std::lock_guard<Mutex> lock(db_test);
  mstatus_t st = STATUS_NOT;
  if (source_ptr != nullptr) {
    if (is_status_ok(st = source_ptr->CheckConnection())) {
      // todo: смешение уровней абстракции
      id_container ids;
      source_ptr->SaveVectorOfRows(models_info, &ids);
      if (is_status_aval(ids.status))
        for (size_t i = 0; i < std::min(models_info.size(), ids.id_vec.size()); ++i)
          models_info[i].id = ids.id_vec[i];
      ids.id_vec.clear();
      ids.status = STATUS_DEFAULT;
      source_ptr->SaveVectorOfRows(calc_info, &ids);
      if (is_status_aval(ids.status))
        for (size_t i = 0; i < std::min(calc_info.size(),ids.id_vec.size()) ; ++i)
          calc_info[i].id = ids.id_vec[i];
      source_ptr->SaveVectorOfRows(result);
    } else {
      Logging::Append(io_loglvl::debug_logs, "Ошибка добавления данных в БД "
          "для смеси: \"" + mixname + "\" при проверке соединения с БД");
    }
  } else {
    Logging::Append(io_loglvl::debug_logs, "Ошибка добавления данных в БД "
        "для смеси: \"" + mixname + "\" - пустой указатель на БД");
  }
  return st;
}

void CalculationSetup::gasmix_models_map::initInfoBinding() {
  calculation_info ci;
  ci.SetCurrentTime();
  assert(models_info.size() == calc_info.size());
  for (size_t i = 0; i < models_info.size(); ++i) {
    calc_info[i].SetModelInfo(&models_info[i]);
    models_info[i].model_p->SetCalculationSetup(&calc_info[i]);
  }
}

void CalculationSetup::gasmix_models_map::calculatePoint(const parameters &p) {
  for (auto mp = models.begin(); mp != models.end(); ++mp) {
    if (mp->second) {
      if (mp->second->IsValid(p)) {
        mp->second->SetVolume(p.pressure, p.temperature);
        result.push_back(mp->second->GetStateLog().SetCalculationInfo(
            mp->second->GetCalculationInfo()));
        break;
      }
    }
  }
}

CalculationSetup::CalculationSetup(std::shared_ptr<file_utils::FileURLRoot> &root,
    const std::string &filepath)
  : root_(root) {
  init_data_.reset(new calculation_setup(root_));
  if (init_data_ != nullptr) {
    // todo: почти неиспользуемая переменная path
    auto path = root_->CreateFileURL(filepath);
    initSetup(&path);
  } else {
    if (root_ != nullptr) {
      error_.SetError(ERROR_FILE_IN_ST, "Ошибка инициализации файла"
          " конфигурации расчёта:\n\troot: " + root_->CreateFileURL("").GetURL() +
          " \tфайл: " + filepath);
    } else {
      error_.SetError(ERROR_FILE_IN_ST, "Ошибка инициализации файла"
        " конфигурации расчёта:\n\troot директория не задана " );
    }
  }
}

void CalculationSetup::Calculate() {
  // Блокировать изменение данных пока не проведены расчёты
  std::lock_guard lock(gasmixes_lock_);
  std::vector<std::future<void>> future_points;
  for (auto gmix: gasmixes_) {
    if (!gmix.second->models.empty())
      future_points.push_back(std::async(std::launch::async, &CalculationSetup::
          gasmix_models_map::CalculatePoints, gmix.second.get(), points_));
  }
  for (auto &fp: future_points)
    // расчитать точки
    fp.get();
}

mstatus_t CalculationSetup::AddToDatabase(DBConnectionManager *source_ptr) {
  // std::lock_guard lock(gasmixes_lock_);
  std::vector<std::future<mstatus_t>> future_points;
  mstatus_t st = STATUS_OK;
  for (auto gmix: gasmixes_)
    future_points.push_back(std::async(std::launch::async, &CalculationSetup::
        gasmix_models_map::AddToDatabase, gmix.second.get(), source_ptr));

  for (auto &fp: future_points)
    // если были ошибки при добавлении данных в бд, отметим это
    if (!is_status_ok(fp.get()))
      st = STATUS_NOT;
  return st;
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

mstatus_t CalculationSetup::initModel(CalculationSetup::gasmix_models_map *models_map,
    time_t datetime, const model_str &ms, file_utils::FileURLRoot *root,
    const std::string &filemix) {
  std::shared_ptr<modelGeneral> m_ptr = std::shared_ptr<modelGeneral>(
      ModelsCreator::GetCalculatingModel(ms, root, filemix));
  mstatus_t st = STATUS_OK;
  if (m_ptr) {
    // обновим структуру моделей models
    models_map->models.emplace(m_ptr->GetPriority(), m_ptr);
    // добавить структуру информации в models_info
    models_map->models_info.emplace(models_map->models_info.end(), model_info(
        model_info::GetDefault().SetModelStr(ms).SetModelPtr(m_ptr.get())));
    // добавить структуру информации о расчёте в calc_info
    models_map->calc_info.emplace(models_map->calc_info.end(),
        calculation_info().SetDateTime(&datetime));
    // инициализировать данные структуры расчёта
  } else {
    st = STATUS_HAVE_ERROR;
    Logging::Append(ERROR_INIT_NULLP_ST,
        "ошибка создания расчётной модели для файла: " + filemix +
        "\n\t" + modelGeneral::GetModelShortInfo(ms.model_type).GetString());
  }
  return st;
}

merror_t CalculationSetup::initSetup(file_utils::FileURL *filepath_p) {
  // Прячем указатель на calculation_setup
  //   в класс-строитель
  CalculationSetupBuilder builder(init_data_.get());
  // Инициализируем ридер по адресу пути к файлу и классу строителю выше
  std::unique_ptr<ReaderSample<pugi::xml_node, calculation_node<pugi::xml_node>,
      CalculationSetupBuilder>> rs(ReaderSample<pugi::xml_node,
      calculation_node<pugi::xml_node>, CalculationSetupBuilder>::
      Init(filepath_p, &builder));
  merror_t error = ERROR_SUCCESS_T;
  if (rs) {
    rs->InitData();
    if (!rs->GetErrorCode()) {
      // инициализировать расчётные модели
      if (initData()) {
        // возможны не критические ошибки
        error = error_.GetErrorCode();
        error_.LogIt();
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
  //gasmix_models_map models_map;
  merror_t error = ERROR_SUCCESS_T;
  for (auto file: init_data_->gasmix_files) {
    mstatus_t st = STATUS_OK;
    auto emplace_pair = gasmixes_.emplace(file,
        std::shared_ptr<gasmix_models_map>(new gasmix_models_map));
    if (emplace_pair.second) {
      /* todo: название смеси вообще-то подцепляется в xml,
       *   имя файла не является названием смеси  */
      emplace_pair.first->second->mixname = file;
      auto path_str = root_->CreateFileURL(file).GetURL();
      std::time_t dt = time(0);
      std::vector<std::future<mstatus_t>> future_models;
      for (auto m: init_data_->models)
        future_models.push_back(std::async(std::launch::async,
            &CalculationSetup::initModel, emplace_pair.first->second.get(), dt,
            modelGeneral::GetModelShortInfo(m), root_.get(), path_str));
      for (auto &fm: future_models) {
        // если хотя бы одна из моделей не проинициализирована
        //   отметим что была ошибка
        if (!is_status_ok(fm.get()))
          st = STATUS_NOT;
      }
      // во время инициализации моделей была ошибка,
      //   проверим если с чем работать
      if (!is_status_ok(st)) {
        if (emplace_pair.first->second->models.size() == 0) {
          Logging::Append(ERROR_INIT_T, "Ошибка инициализации расчёта для " +
              file + " не проинициализирована ни одна модель.");
          error = ERROR_INIT_T;
        }
      }
    }
  }
  if (!gasmixes_.empty()) {
    // "скопировать" расчётные точки
    points_ = std::vector<parameters>(std::move(init_data_->points));
    // удаляем данные сетапа
    // todo: зачем? или хотя бы перенести
    // init_data_ = nullptr;
  } else {
    status_ = STATUS_HAVE_ERROR;
    error = error_.SetError(ERROR_INIT_T, "Не проинициализированы "
        "расчётные данные");
  }
  return error;
}
