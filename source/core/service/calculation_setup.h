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


namespace asp_db {
  class DBConnectionManager;
}
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
  public:
    /**
     * \brief Рассчитать точки
     * */
    void CalculatePoints(const std::vector<parameters> &points);
    /**
     * \brief Добавить данные в БД
     * \param source_ptr Указатель на хранилище данных
     * \return Результат добавления
     * */
    mstatus_t AddToDatabase(asp_db::DBConnectionManager *source_ptr) const;

  private:
    /**
     * \brief Инициализировать `*_info` контейнеры
     * */
    void initInfo();
    /**
     * \brief Рассчитать параметры в точке `p` по наиболее
     *   приоритетной модели
     * */
     void calculatePoint(const parameters &p);

  public:
    ErrorWrap error;
    mstatus_t status = STATUS_DEFAULT;
    /**
     * \brief Название смеси
     * */
    std::string mixname;
    /**
     * \brief Сортированный контейнер расчётных моделей
     * */
    std::multimap<priority_var, std::shared_ptr<modelGeneral>> models;
    /**
     * \brief Ссылка на текущую используемую модель
     * */
    modelGeneral *current_model = nullptr;
    /**
     * \brief Копия установленных параметров
     * */
    parameters params_copy;

    /* Данные расчёта */
    /**
     * \brief Информация о моделях
     * */
    std::vector<model_info> models_info;
    /**
     * \brief Информация о расчёте
     * */
    std::vector<calculation_info> calc_info;
    /**
     * \brief Результаты расчёта
     * */
    std::vector<calculation_state_log> result;

    /* Динамика */
    /**
     * \brief Данные расчётов динамики
     * \note В динамике отслеживается история расчётов,
     *   и вероятна последовательная алгоритмизация
     * \todo Доделать, или хотя бы продумать как это могло бы выглядеть
     * */
    dynamic_data dyn_data;
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

  /**
   * \brief Рассчитать инициализированные точки
   * */
  void Calculate();
  /**
   * \brief Сохранить рассчитанные параметры в базе данных
     * \param source_ptr Указатель на хранилище данных
   * */
  mstatus_t AddToDatabase(asp_db::DBConnectionManager *source_ptr);
  /* Данные расчёта */
  /**
   * \brief Получить вектор результатов
   * */
  std::vector<calculation_state_log> &GetCalculationResult();
  /**
   * \brief Получить вектор информации о расчёте
   * */
  std::vector<calculation_info> &GetCalculationInfo();

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
   * \brief Инициализировать расчётную модель и сопутствующие
   *   данные, добавить их структуру хранения моделей models_map
   * \param models_map Указатель на структуру хранения моделей
   * \param datetime Дата и время
   * \note Собственно условночистая функция для распараллеливания
   * */
  static mstatus_t initModel(gasmix_models_map *models_map, std::time_t datetime,
      const model_str &ms, const std::string &filemix);

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
   * \brief Мьютекс на изменение данных для обсчёта
   * */
  Mutex gasmixes_lock_;
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
   * \brief Точки расчёта (p, t)
   * */
  std::vector<parameters> points_;

  /* state */
  /**
   * \brief Данные сохраненны в БД
   * */
  bool saved_ = false;
};
using Calculations = std::map<int, CalculationSetup>;

#endif  // !_CORE__SERVICE__CALCULATION_SETUP_H_
