#ifndef _CORE__COMMON__COMMON_H_
#define _CORE__COMMON__COMMON_H_

#include <string>

#include <stdint.h>

// separate calculation of parameters of mix 
//   просто интересно
// #define GAS_MIX_VARIANT  // -- total incorrect
/* TODO: remove this define */
#define BY_PSEUDO_CRITIC

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


#define MARK_MASK             0x0000FF00
#define GAS_MIX_MARK          0x00000100
#define PSEUDO_CRITIC_MARK    0x00000200
#define BINASSOC_COEFS_MARK   0x00000300
// ГОСТ 30319.3-2015
#define GAS_NG_GOST_MARK      0x00000400

//  math defines
#define FLOAT_ACCURACY        0.00001
#define DOUBLE_ACCURACY       0.000000001

// logging defines
#define DEFAULT_LOGLVL        0x01
#define DEBUG_LOGLVL          0x0f

#if defined(OS_WIN)
#  define PATH_SEPARATOR '\\'
#elif defined(OS_NIX)
#  define PATH_SEPARATOR '/'
#else
#  error **undef platform**
#endif

typedef uint64_t gas_marks_t;

enum class rg_model_t : uint64_t {
  IDEAL_GAS = MODEL_IDEAL_GAS,
  REDLICH_KWONG2 = MODEL_REDLICH_KWONG,
  PENG_ROBINSON = MODEL_PENG_ROBINSON,
  NG_GOST = MODEL_NG_GOST
};

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
