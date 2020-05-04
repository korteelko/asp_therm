/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#ifndef _CORE__MODELS__MODELS_CONFIGURATIONS_H_
#define _CORE__MODELS__MODELS_CONFIGURATIONS_H_

#include "calculation_info.h"
#include "common.h"
#include "ErrorWrap.h"

#include <ctime>
#include <memory>
#include <string>
#include <vector>


/* CONFIGURATION PARAMETERS (XML or JSON)
 * // MODELS_STR - структура описывающая модель
 * MODELS_STR {
 *   TYPE : INT
 *   SUBTYPE : INT
 *   NAME : STRING
 *   VERSION_MAJOR : INT
 *   VERSION_MINOR : INT
 * }
 *
 * DATABASE_CONFIGURATION {
 *   CLIENT : STRING
 *   // DB_PATH : STRING
 *   NAME : STRING
 *   USERNAME : STRING
 *   PASSWORD : STRING
 *   HOST : STRING
 *   PORT : STRING
 * }
 *
 * PROGRAM_CONFIGURATION:
 * - DEBUG_MODE : BOOL
 * - PSEUDOCRITIC : BOOL
 * - INCLUDE_ISO_20765 : BOOL
 * - LOG_LEVEL : INT
 * - LOG_FILE : STRING
 * - DATABASE : DATABASE_CONFIGURATION
 * // - MODELS : MODELS_STR[] - move to calculation.json
*/

/** \brief Приоритет использования модели(относительно
  *   других моделей
  * \note Для организации нескольких моделей в мапе */
struct model_priority {
private:
  /** \brief Численное значение приоритета [-1, 127]
    *   127 - максимальный;
    *   0 - идеальный газ;
    *   -1 - использование модели недопустимо - неверные
    *     константные данные, например мольный состав. */
  priority_var priority;
  /** \brief Приоритет не специализирован и будет задан
    *   посредством конфигурации модели(установлен приоритет
    *   по умлчанию) */
  bool is_specified;
  /** \brief Проводить расчёты игнорируя ограничения модели
    * \note необходимо, скорее из "онтологических" соображений,
    *   поэтому по умолчанию false :) */
  bool is_forced = false;

public:
  model_priority();
  explicit model_priority(priority_var priority);
  /** \brief Получить числовое значение приоритета */
  priority_var GetPriority() const;
  /** \brief Используется значение не по умолчанию */
  bool IsSpecified() const;
  /** \brief Установлен флаг игнорирования границ применения модели */
  bool IsForced() const;
  /* \brief Использование модели допустимо(пока не определился как
    *   правильно использовать эту функцию) */
  // bool IsAvailableModel() const;

  bool operator<(const model_priority &s);
};
inline priority_var model_priority::GetPriority() const { return priority; }
inline bool model_priority::IsSpecified() const { return is_specified; }
inline bool model_priority::IsForced() const { return is_forced; }
// inline bool model_priority::IsAvailableModel() const { return priority != -1; }


/** \brief Структура идентификации модели(уравнения реального газа)
  *   параметры прописываются в классе параметров модели
  *   методом GetModelStr */
struct model_str {
  /** \brief Тип модели */
  rg_model_id model_type;
  /** \brief Мажорная версия модели */
  int32_t vers_major;
  /** \brief Минорная версия */
  int32_t vers_minor;
  /** \brief Короткая информация о модели */
  std::string short_info;

public:
  model_str(rg_model_id ml, int32_t vmaj, int32_t vmin,
      const std::string &info);
};

/** \brief конфигурация прораммы,
  *   общая для запущенной конфигурации */
struct program_configuration {
public:
  /** \brief информация доступных моделей */
  calculation_configuration calc_cfg;
  /** \brief уровень логирования */
  io_loglvl log_level;
  /** \brief файл логирования */
  std::string log_file;

public:
  program_configuration();
  /** \brief изменить параметр(поле), соответствующий 'param_str',
    *   значение параметра соответствует переданному в строке 'param_value'
    * \param param_strtpl текстовый шаблон поля
    * \param param_value значение параметра */
  merror_t SetConfigurationParameter(const std::string &param_strtpl,
      const std::string &param_value);
};

/** \brief строка для добавления в БД */
struct model_info {
  enum model_info_flags {
    f_empty = 0x00,
    f_model_type = 0x01,
    f_model_subtype = 0x02,
    f_vers_major = 0x04,
    f_vers_minor = 0x08,
    f_short_info = 0x10,
    /* спорный вопрос насчёт этого */
    f_model_id = 0x20,
    f_full = 0x3F
  };
  /** \brief Уникальный id строки из базы данных */
  int32_t id = -1;
  /** \brief Информация о модели */
  model_str short_info;
  // dyn_setup dynamic_vars;
  /** \brief Инициализированные поля, для операций SELECT, UPDATE, INSERT */
  uint32_t initialized = f_empty;

public:
  /** \brief Получить стандартную структуру model_info , с пустыми полями */
  static model_info GetDefault();
};

#endif  // !_CORE__MODELS__MODELS_CONFIGURATIONS_H_
