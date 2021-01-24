/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020-2021 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#ifndef _CORE__COMMON__ATHERM_COMMON_H_
#define _CORE__COMMON__ATHERM_COMMON_H_

#include <sstream>

#include "Common.h"
#include "merror_codes.h"

// original of CSI gost 30319-2015
/* todo: remove to config file
 *   also: дефайн не просто так - если вычисления должны быть
 *   исполнены исключительно по ГОСТ модели, то его вообще лучше
 *   убрать */
#if defined(ISO_20765)
// natural gas may contain components
//   that can be associated with one of GAS_TYPE...
//   look ISO 20765 E.1
#define ASSIGNMENT_TRACE_COMPONENTS
#endif  // ISO_20765

#define MODEL_MASK 0x0000000F
#define BINODAL_MODEL_MASK 0x000000F0
/* TODO: realize this with functions */
#define BINODAL_MODEL_SHIFT 4
#define MARK_MASK_SHIFT 8

#define MODEL_EMPTY 0x00000000
#define MODEL_IDEAL_GAS 0x00000001
#define MODEL_REDLICH_KWONG 0x00000002
#define MODEL_PENG_ROBINSON 0x00000003
#define MODEL_NG_GOST 0x00000004

/* Флажки конфигурации газовой смеси */
#define MARK_MASK 0x0000FF00
#define GAS_MIX_MARK 0x00000100
#define GAS_NG_GOST_MARK 0x00000400
#define GAS_NG_ISO_MARK 0x00000800

/*  приоритет уравнения сосотяния по умолчанию,
 *    если не задан при инициализации */
/** \brief целочисленный тип приоритета [-1, 127] */
typedef int8_t priority_var;
#define DEF_PRIOR_MINIMUM -1
#define DEF_PRIOR_IDEAL_GAS 0
#define DEF_PRIOR_RK 50
#define DEF_PRIOR_RKS 75
#define DEF_PRIOR_PR 60
#define DEF_PRIOR_PR_Bin 75
#define DEF_PRIOR_GOST 100
#define DEF_PRIOR_GOST_ISO 110
#define DEF_PRIOR_MAXIMUM 127

typedef uint64_t gas_marks_t;
/** \brief Проверить, установлен ли флаг использования модели ГОСТ 30319 */
bool HasGostModelMark(const gas_marks_t m);
/** \brief Проверить, установлен ли флаг использования модели ISO */
bool HasGostISO20765Mark(const gas_marks_t m);
/** \brief Проверить, установлен ли флаг смеси газов */
bool HasGasMixMark(const gas_marks_t m);

/** \brief Установить флаг использования модели ГОСТ 30319 */
void AddGostModelMark(gas_marks_t* m);
/** \brief Установить флаг использования модели ISO */
void AddGostISO20765Mark(gas_marks_t* m);
/** \brief Установлен флаг смеси газов */
void AddGasMixMark(gas_marks_t* m);

/** \brief Используемые модели */
enum class rg_model_t : uint64_t {
  /** \brief Заглушка типа модели для дефолтных сетапов */
  EMPTY = MODEL_EMPTY,
  /** \brief Модель идеального газа
   * \note just for fun */
  IDEAL_GAS = MODEL_IDEAL_GAS,
  /** \brief Уравнение состояния Редлиха-Квонга
   * \note В классической постановке и в модификации Соаве */
  REDLICH_KWONG = MODEL_REDLICH_KWONG,
  /** \brief Уравнение состояние Пенга-Робинсона, в нескольких модификациях */
  PENG_ROBINSON = MODEL_PENG_ROBINSON,
  /** \brief Не кубическое уравкнение состояния составленное по материалам
   *   ГОСТ 30319 и ISO 20765 */
  NG_GOST = MODEL_NG_GOST
};
typedef int64_t rg_model_subtype;
/* model_subtypes */
/* todo: привязать к моделям */
#define MODEL_SUBTYPE_DEFAULT 0x00000000
/* модификация Соаве для модели Редлиха-Квонга */
#define MODEL_RK_SUBTYPE_SOAVE 0x00000001
/* для модели Пенга-Робинсона - инициализация
 *   параметров смеси ч/з бинодальные коэффициенты */
#define MODEL_PR_SUBTYPE_BINASSOC 0x00000001
/* расширение ГОСТ 30319-2015 */
#define MODEL_GOST_SUBTYPE_ISO_20765 0x00000001
struct rg_model_id {
  rg_model_t type;
  /** \brief subtypenumber(subml_typenumber) - наверное привязаться к
   *   енамам конкретных моделей
   * default 0, т.е. например для Редлиха-Квонга есть модификация
   * Соаве, а для Пенг-Робинсона их не счесть */
  rg_model_subtype subtype;

  rg_model_id() = delete;
  rg_model_id(rg_model_t t, rg_model_subtype subt);

  inline bool operator==(const rg_model_id& r) const {
    return (type == r.type) && (subtype == r.subtype);
  }
};

#endif  // !_CORE__COMMON__ATHERM_COMMON_H_
