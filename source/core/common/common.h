/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#ifndef _CORE__COMMON__COMMON_H_
#define _CORE__COMMON__COMMON_H_

#include <sstream>
#include <string>

#include <stdint.h>

// separate calculation of parameters of mix
//   просто интересно

// original of CSI gost 30319-2015
/* TODO: remove to config file */
#define ISO_20765
#if defined(ISO_20765)
// natural gas may contain components
//   that can be associated with one of GAS_TYPE...
//   look ISO 20765 E.1
#  define ASSIGNMENT_TRACE_COMPONENTS
#endif  // ISO_20765

#define MODEL_MASK            0x0000000F
#define BINODAL_MODEL_MASK    0x000000F0
/* TODO: realize this with functions */
#define BINODAL_MODEL_SHIFT   4
#define MARK_MASK_SHIFT       8

#define MODEL_IDEAL_GAS       0x00000001
#define MODEL_REDLICH_KWONG   0x00000002
#define MODEL_PENG_ROBINSON   0x00000003
#define MODEL_NG_GOST         0x00000004


/* todo: переделать установку/чтение флажков -
 *   вынести всё в отдельную функцию */
#define MARK_MASK             0x0000FF00
#define GAS_MIX_MARK          0x00000100
// ГОСТ 30319.3-2015
#define GAS_NG_GOST_MARK      0x00000400

//  math defines
#define FLOAT_ACCURACY        0.00001
#define DOUBLE_ACCURACY       0.000000001

// logging defines
#define DEFAULT_LOGLVL        0x01
#define DEBUG_LOGLVL          0x0f

typedef uint32_t mstatus_t;
/** \brief статус при инициализации */
#define STATUS_DEFAULT        0x00000001
/** \brief статус удачного результата операции */
#define STATUS_OK             0x00000002
/** \brief статус неудачного результата операции */
#define STATUS_NOT            0x00000003
/** \brief статус наличия ошибки при выполнении операции */
#define STATUS_HAVE_ERROR     0x00000004

/* todo: rename, remove */
#define FILE_LAST_OBJECT       0x1000

/*  приоритет уравнения сосотяния по умолчанию,
 *    если не задан при инициализации */
/** \brief целочисленный тип приоритета [-1, 127] */
typedef int8_t priority_var;
#define DEF_PRIOR_MINIMUM   -1
#define DEF_PRIOR_IDEAL_GAS 0
#define DEF_PRIOR_RK        50
#define DEF_PRIOR_RKS       75
#define DEF_PRIOR_PR        60
#define DEF_PRIOR_PR_Bin    75
#define DEF_PRIOR_GOST      100
#define DEF_PRIOR_GOST_ISO  110
#define DEF_PRIOR_MAXIMUM   127


/* дефайны из файла CMakeLists.txt */
#if defined(BYCMAKE_DEBUG)
#  define _DEBUG
#endif  // BYCMAKE_DEBUG
#if defined(BYCMAKE_WITH_PUGIXML)
#  define WITH_PUGIXML
#endif  // BYCMAKE_WITH_PUGIXML
#if defined(BYCMAKE_WITH_RAPIDJSON)
#  define WITH_RAPIDJSON
#endif  // BYCMAKE_WITH_RAPIDJSON
#if defined(BYCMAKE_WITH_POSTGRESQL)
#  define WITH_POSTGRESQL
#endif  // BYCMAKE_WITH_POSTGRESQL
#if defined(BYCMAKE_TESTS_ENABLED)
#  define TESTS_ENABLED
#endif  // BYCMAKE_TESTS_ENABLED
#if defined(BYCMAKE_CXX17)
#  define CXX17
#endif  // BYCMAKE_CXX17


#if defined(OS_WIN)
#  define PATH_SEPARATOR '\\'
#elif defined(OS_NIX)
#  define PATH_SEPARATOR '/'
#else
#  error **undef platform**
#endif


typedef uint64_t gas_marks_t;
/** \brief Проверить, установлен ли флаг использования модели ГОСТ 30319 */
bool HasGostModelMark(const gas_marks_t m);
/** \brief Проверить, установлен ли флаг смеси газов */
bool HasGasMixMark(const gas_marks_t m);

/** \brief Установить флаг использования модели ГОСТ 30319 */
void AddGostModelMark(gas_marks_t *m);
/** \brief Установлен флаг смеси газов */
void AddGasMixMark(gas_marks_t *m);

/** \brief Используемые модели */
enum class rg_model_t : uint64_t {
  IDEAL_GAS = MODEL_IDEAL_GAS,
  REDLICH_KWONG = MODEL_REDLICH_KWONG,
  PENG_ROBINSON = MODEL_PENG_ROBINSON,
  NG_GOST = MODEL_NG_GOST
};
typedef int64_t rg_model_subtype;
/* model_subtypes */
/* todo: привязать к моделям */
#define MODEL_SUBTYPE_DEFAULT        0x00000000
/* модификация Соаве для модели Редлиха-Квонга */
#define MODEL_RK_SUBTYPE_SOAVE       0x00000001
/* для модели Пенга-Робинсона - инициализация
 *   параметров смеси ч/з бинодальные коэффициенты */
#define MODEL_PR_SUBTYPE_BINASSOC    0x00000001
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
};

typedef enum {
  /** \brief no messages */
  no_log = 0,
  /** \brief only errors */
  err_logs = DEFAULT_LOGLVL,
  /** \brief warning, errors */
  warn_logs,
  /** \brief all mesages, default for debug */
  debug_logs = DEBUG_LOGLVL
} io_loglvl;

/** \brief Вывести целочисленное значение в шестнадцеричном формате */
template <typename Integer,
    typename = std::enable_if_t<std::is_integral<Integer>::value>>
std::string hex2str(Integer hex) {
  std::stringstream hex_stream;
  hex_stream << "0x" << std::hex << hex;
  return hex_stream.str();
}

/** \brief Проверить допустимость текущего состояния статуса
  * \return true если st == STATUS_DEFAULT или st == STATUS_OK */
inline bool is_status_aval(mstatus_t status) {
  return status == STATUS_DEFAULT || status == STATUS_OK;
}
/** \brief Проверить валидность статуса
  * \return true если st == STATUS_OK */
inline bool is_status_ok(mstatus_t status) {
  return status == STATUS_OK;
}
/** \brief Строка str заканчивается подстрокой ending */
inline bool ends_with(const std::string &str, const std::string &ending) {
  if (ending.size() > str.size())
    return false;
  return std::equal(ending.rbegin(), ending.rend(), str.rbegin());
}
/** \brief Обрезать пробелы с обоих концов */
std::string trim_str(const std::string &str);
/** \brief Проверить что объект файловой системы(файл, директория, соккет,
  *   линк, character_dev) существует */
bool is_exist(const std::string &path);
/** \brief Вернуть путь к директории содержащей файл */
std::string dir_by_path(const std::string &path);

#endif  // !_CORE__COMMON__COMMON_H_
