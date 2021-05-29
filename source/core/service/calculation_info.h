/**
 * asp_therm - implementation of real gas equations of state
 * ===================================================================
 * * calculation_info *
 *   В файле описан функционал управления расчётами:
 *     - Общие параметры из файла конфигурации программы. Вывод
 *   отладочной информации, используемые модификации уравнений состояния.
 *     - Структура параметров расчёта для добавления в БД
 * ===================================================================
 *
 * Copyright (c) 2020-2021 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#ifndef _CORE__SERVICE__CALCULATION_INFO_H_
#define _CORE__SERVICE__CALCULATION_INFO_H_

#include "asp_utils/ErrorWrap.h"
#include "asp_utils/FileURL.h"
#include "atherm_common.h"
#include "gas_description.h"

struct model_info;

/** \brief Конфигурация запуска программы */
struct calculation_configuration {
  /** \brief Флаг вывода отладочной информации
   * \default true */
  bool is_debug_mode = true;
  /** \brief Флаг использования классической модели Редлиха-Квонга
   * \default false */
  bool rk_enable_origin_mod = false;
  /** \brief Флаг использования модификации Соаве
   *   для уравнения состояния Редлиха-Квонга
   * \default true */
  bool rk_enable_soave_mod = true;
  /** \brief Флаг инициализации модели Пенга-Робинсона через
   *   коэфициенты бинарного взаимодействия
   * \default true */
  bool pr_enable_by_binary_coefs = true;
  /** \brief Флаг использования 'ISO 20665' поверх 'ГОСТ 30319-3'
   *    для ng_gost модели
   * \default true */
  bool enable_iso_20765 = true;

 public:
  /** \brief Вывод отладочной информации */
  bool IsDebug() const;
  /** \brief Использование классической модели Редлиха-Квонга */
  bool RK_IsEnableOriginMod() const;
  /** \brief Использование модификации Соаве
   *   для уравнения состояния Редлиха-Квонга */
  bool RK_IsEnableSoaveMod() const;
  /** \brief Использование модели Пенга-Робинсона с расширением через
   *   коэфициенты бинарного взаимодействия */
  bool PR_IsEnableByBinaryCoefs() const;
  /** \brief Использование 'ISO 20665' поверх 'ГОСТ 30319-3'
   *    для ng_gost модели */
  bool IsEnableISO20765() const;
};

/**
 * \brief Структура для добавления в базу данных
 * */
struct calculation_info {
 public:
  calculation_info();
  calculation_info(std::time_t dt);

  /**
   * \brief Установить время/дату
   * */
  calculation_info& SetDateTime(std::time_t* dt);
  /**
   * \brief Установить ссылку на используемую модель
   * */
  calculation_info& SetModelInfo(const model_info* mi);
  /**
   * \brief Установить имя файла газовой смеси
   * */
  calculation_info& SetGasmixFile(const std::string& gasmix);
  /**
   * \brief Установить текущие значения времени и даты
   * */
  calculation_info& SetCurrentTime();
  /**
   * \brief Установить дату
   * \param date дата в формате 'yyyy/mm/dd'
   * */
  mstatus_t SetDate(const std::string& date);
  /**
   * \brief Установить время
   * \param time время в формате 'hh:mm'
   * */
  mstatus_t SetTime(const std::string& time);

  /**
   * \brief Получить строку даты в формате 'yyyy/mm/dd'
   * */
  std::string GetDate() const;
  /**
   * \brief Получить строку времени в формате 'hh:mm:ss'
   * */
  std::string GetTime() const;

 public:
  enum calculation_info_flags {
    f_empty = 0x00,
    f_model_id = 0x01,
    f_date = 0x02,
    f_time = 0x04,
    f_gasmix = 0x08,
    f_calculation_info_id = 0x10,
    f_full = 0x1f
  };
  /**
   * \brief Уникальный id строки расчёта из базы данных
   * */
  int32_t id = -1;
  /**
   * \brief Уникальный id модели из базы данных
   * */
  int32_t model_id = -1;
  /**
   * \brief Указатель на конфигурацию расчёта
   * */
  // const calculation_configuration *configuration = nullptr;
  /**
   * \brief Указатель на модель
   * */
  const model_info* model = nullptr;
  /**
   * \brief Имя файла смеси
   * */
  std::string gasmix_file;
  /**
   * \brief Время и дата
   * */
  std::time_t datetime;
  /**
   * \brief Иницианилизированные поля
   * */
  uint32_t initialized = f_empty;
};

/** \brief информация о предыдущих расчётах */
/* todo: сейчас это заглушка */
struct dynamic_data {
  /** \brief не учитывать историю предыдущих расчётов */
  /* true если мы считаем каким-то пошаговым алгоритмом */
  bool have_history = false;

  /* \brief история расчёта */
  // std::vector<log_calc> history;
};

#endif  // !_CORE__SERVICE__CALCULATION_INFO_H_
