/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#ifndef _CORE__MODELS__CALCULATION_INFO_H_
#define _CORE__MODELS__CALCULATION_INFO_H_

#include "common.h"
#include "gas_description.h"
#include "ErrorWrap.h"

#include <map>


struct model_info;

/** \brief Конфигурация запуска программы */
struct calculation_configuration {
  /** \brief флаг вывода отладочной информации
    * \default true */
  bool is_debug_mode = true;
  /** \brief флаг использования классической модели Редлиха-Квонга
    * \default false */
  bool rk_enable_origin_mod = false;
  /** \brief флаг использования модификации Соаве
    *   для модели Редлиха-Квонга
    * \default true */
  bool rk_enable_soave_mod = true;
  /** \brief флаг инициализации модели Пенга-Робинсона через
    *   коэфициенты бинарного взаимодействия
    * \default true */
  bool pr_enable_by_binary_coefs = true;
  /** \brief флаг использования 'ISO 20665' поверх 'ГОСТ 30319-3'
    *    для ng_gost модели
    * \default true */
  bool enable_iso_20765 = true;

public:
  bool IsDebug() const;
  bool RK_IsEnableOriginMod() const;
  bool RK_IsEnableSoaveMod() const;
  bool PR_IsEnableByBinaryCoefs() const;
  bool IsEnableISO20765() const;
};

/** \brief структура содержащая данные инициализации расчёта */
struct calculation_setup {
  /** \brief файл иниициализации газовой смеси */
  std::string gasmix_file;
  /** \brief файл иниициализации используемых моделей */
  std::string models_file;
  /** \brief файл описывающий области расчёта,
    *   соответствующие им модели */
  std::string calculate_file;
};

/* сейчас посто заглушка
 * todo: добавить calculation configuration */
/** \brief Структура для добавления в базу данных */
struct calculation_info {
public:
  calculation_info();

  /** \brief Установить время/дату */
  void SetDateTime(std::time_t *dt);

  /** \brief Получить строку даты в формате 'yyyy/mm/dd' */
  std::string GetDate() const;
  /** \brief Получить строку времени в формате 'hh:mm:ss' */
  std::string GetTime() const;

public:
  enum calculation_info_flags {
    f_empty = 0x00,
    f_model_id = 0x01,
    f_date = 0x02,
    f_time = 0x04,
    f_full = 0x07
  };
  /** \brief Уникальный id строки из базы данных */
  int32_t id = -1;
  /** \brief Указатель на конфигурацию расчёта */
  const calculation_configuration *configuration = nullptr;
  /** \brief Указатель на модель */
  model_info *model = nullptr;
  /** \brief Время и дата */
  std::time_t datetime;
  /** \brief Иницианилизированные поля */
  calculation_info_flags initialized = f_empty;
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

/** \brief конфигурация расчёта */
/* todo: remove to separate file */
class CalculationSetup {
public:
  CalculationSetup();
  CalculationSetup(const calculation_setup &cs);
#if !defined(DATABASE_TEST)
#  ifdef _DEBUG
  merror_t AddModel(std::shared_ptr<modelGeneral> &mg);
#  endif  // _DEBUG
  /** \brief Проверить допустимость валидность используемой
    *   модели current_model_ */
  mstatus_t CheckCurrentModel();
#endif  // !DATABASE_TEST
  merror_t SetModel(int model_key);
  merror_t GetError() const;

private:
  merror_t init_setup();
  void swap_model();

private:
  ErrorWrap error_;
  mstatus_t status_;
  /** \brief копия установленных параметров */
  parameters params_copy_;
#ifdef _DEBUG
  /** \brief список используемых моделей для расчёта */
  std::multimap<priority_var, std::shared_ptr<modelGeneral>> models_;
#else
  /** \brief список используемых моделей для расчёта */
  std::multimap<priority_var, std::unique_ptr<modelGeneral>> models_;
#endif  // _DEBUG
  /** \brief ссылка на текущую используемую модель */
  modelGeneral *current_model_;
  /** \brief данные инициализации расчёта */
  calculation_setup init_data_;
  /** \brief данные расчётов динамики
    * \note в динамике отслеживается история расчётов,
    *   и вероятна последовательная алгоритмизация */
  dynamic_data dyn_data_;
};
using Calculations = std::map<int, CalculationSetup>;

#endif  // !_CORE__MODELS__CALCULATION_INFO_H_
