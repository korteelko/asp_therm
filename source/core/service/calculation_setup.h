/**
 * asp_therm - implementation of real gas equations of state
 * ===================================================================
 * * calculation_setup *
 *   Модуль обрабатывающий конфигурации расчётов.
 *     Здесь инициализируются, храняться и обрабатываются наборы данных,
 *     описывающие расчёты.
 *   Конфигурация расчёта предполагает описание области расчёта,
 *     состав газовых смесей, используемый инструментарий.
 * ===================================================================
 *
 * Copyright (c) 2020 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#ifndef _CORE__SERVICE__CALCULATION_SETUP_H_
#define _CORE__SERVICE__CALCULATION_SETUP_H_

#include "atherm_common.h"
#include "calculation_info.h"
#include "Common.h"
#include "ErrorWrap.h"
#include "model_general.h"
#include "ThreadWrap.h"

#include <map>


/**
 * \brief Набор данных конфигурации расчёта
 * */
struct calculation_setup {
public:
  calculation_setup(std::shared_ptr<file_utils::FileURLRoot> &root);

public:
  /**
   * \brief Корневая директория текстовых файлов
   * \note shared_ptr можно на обычный указатель заменить, но так погибче
   * */
  std::shared_ptr<file_utils::FileURLRoot> root;
  /**
   * \brief Используемые модели
   * */
  std::vector<rg_model_id> models;
  /**
   * \brief Файлы иниициализации газовых смесей
   * */
  std::vector<std::string> gasmix_files;
  /**
   * \brief Точки расчёта
   * \note По параметрам давления и температуры найти плотность
   * */
  std::vector<parameters> points;
};


/**
 * \brief Класс инициализации конфигурации расчёта - модели,
 *   компоненты, входные точки
 * */
class CalculationSetup {
public:
  /**
   * \brief Сетап для обсчёта газовой смеси
   * */
  struct gasmix_models_map {
    /**
     * \brief Сортированный контейнер расчётных моделей
     * */
    std::multimap<priority_var, std::shared_ptr<modelGeneral>> models;
    /**
     * \brief Ссылка на текущую используемую модель
     * */
    modelGeneral *current_model;
    /**
     * \brief Копия установленных параметров
     * */
    parameters params_copy_;
  };

public:
  CalculationSetup(std::shared_ptr<file_utils::FileURLRoot> &root,
      const std::string &filepath);

  virtual ~CalculationSetup() = default;
  CalculationSetup(CalculationSetup &&) = default;

  // бикоз не надо и лень
  CalculationSetup(const CalculationSetup &) = delete;
  CalculationSetup &operator=(const CalculationSetup &) = delete;
  CalculationSetup &operator=(CalculationSetup &&) = delete;


#if !defined(DATABASE_TEST)
#  ifdef _DEBUG
  merror_t AddModel(std::shared_ptr<modelGeneral> &mg);
#  endif  // _DEBUG
  /** \brief Проверить допустимость(валидность) используемой
    *   модели current_model_ */
  mstatus_t CheckCurrentModel();
#endif  // !DATABASE_TEST
  merror_t SetModel(int model_key);
  merror_t GetError() const;

protected:
  /**
   * \brief Инициализировать конфигурацию расчёта
   * */
  merror_t initSetup(file_utils::FileURL *filepath_p);
  /**
   * \brief Инициализировать данные класса по данным
   *   структуры calculation_setup
   * */
  merror_t initData();
  /**
   * \brief Инициализировать точки расчёта
   * */
  merror_t initPoints();
  /**
   * \brief Переключить используемую модель на самую приоритетную
   *   из допустимых
   * */
  // void swapModel();

protected:
  ErrorWrap error_;
  mstatus_t status_ = STATUS_DEFAULT;
  /**
   * \brief Данные инициализации расчёта
   * \note Структура нужна только для инициализации
   * */
  std::unique_ptr<calculation_setup> init_data_ = nullptr;
  /**
   * \brief Список смесей для обсчёта
   * \todo Здесь мы параллелим
   * */
  std::map<std::string, gasmix_models_map> gasmixes_;
  /**
   * \brief Точки расчёта
   * */
  std::vector<parameters> points_;
  /**
   * \brief Данные расчётов динамики
   * \note В динамике отслеживается история расчётов,
   *   и вероятна последовательная алгоритмизация
   * \todo Доделать, или хотя бы продумать как это могло бы выглядеть
   * */
  dynamic_data dyn_data_;
  /**
   * \brief Мьютекс на изменение данных для обсчёта
   */
   Mutex gasmixes_lock_;
};
using Calculations = std::map<int, CalculationSetup>;

#endif  // !_CORE__SERVICE__CALCULATION_SETUP_H_
