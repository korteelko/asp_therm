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
#define PSEUDO_CRITIC_MARK    0x00000200
// ГОСТ 30319.3-2015
#define GAS_NG_GOST_MARK      0x00000400

//  math defines
#define FLOAT_ACCURACY        0.00001
#define DOUBLE_ACCURACY       0.000000001

// logging defines
#define DEFAULT_LOGLVL        0x01
#define DEBUG_LOGLVL          0x0f

typedef uint32_t mstatus_t;
/** статус при инициализации */
#define STATUS_DEFAULT        0x00000001
/** как и STATUS_DEFAULT, но для дебажной
  *   dry_run опции */
#define STATUS_DRY_RUN        0x00000002
#define STATUS_OK             0x00000003
#define STATUS_NOT            0x00000004
#define STATUS_HAVE_ERROR     0x00000005

#if defined(OS_WIN)
#  define PATH_SEPARATOR '\\'
#elif defined(OS_NIX)
#  define PATH_SEPARATOR '/'
#else
#  error **undef platform**
#endif

typedef uint64_t gas_marks_t;


/* Используемые модели */
enum class rg_model_t : uint64_t {
  IDEAL_GAS = MODEL_IDEAL_GAS,
  REDLICH_KWONG2 = MODEL_REDLICH_KWONG,
  PENG_ROBINSON = MODEL_PENG_ROBINSON,
  NG_GOST = MODEL_NG_GOST
};
typedef int64_t rg_model_subtype;
/* model_subtypes */
#define MODEL_SUBTYPE_DEFAULT   0x00000000
/* для модели Пенга Робинсона - инициализация
 *   параметров смеси ч/з бинодальные коэффициенты */
#define MODEL_SUBTYPE_BINASSOC  0x00000001
/* расширение ГОСТ 30319-2015 */
#define MODEL_SUBTYPE_ISO_20765 0x00000002


typedef enum {
  no_log     = 0,              /* no messages     */
  err_logs   = DEFAULT_LOGLVL, /* only errors     */
  warn_logs,                   /* warning, errors */
  debug_logs = DEBUG_LOGLVL    /* all mesages, default for debug */
} io_loglvl;


/** \brief Обрезать пробелы с обоих концов */
std::string trim_str(const std::string &str);
/** \brief Проверить что объект файловой системы(файл, директория, соккет,
  *   линк, character_dev) существует */
bool is_exist(const std::string &path);
/** \brief Вернуть путь к директории содержащей файл */
std::string dir_by_path(const std::string &path);
/** \brief Вернуть путь к директории содержащей файл */
std::string hex2str(int hex);

#endif  // !_CORE__COMMON__COMMON_H_
