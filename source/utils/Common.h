/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#ifndef _UTILS__COMMON_H_
#define _UTILS__COMMON_H_

#include <string>
#include <sstream>

#include <stdint.h>


#define ERROR_SUCCESS_T     0x0000
#define ERROR_GENERAL_T     0x0001

#if defined(BYCMAKE_DEBUG)
/** \brief Режим отладки */
#  define _DEBUG
#endif  // BYCMAKE_DEBUG
#if defined(BYCMAKE_CXX17)
/** \brief Использовать стандарт C++17 */
#  define CXX17
#endif  // BYCMAKE_CXX17
#if defined(BYCMAKE_WITH_POSTGRESQL)
/** \brief Использовать библиотеку libpqxx */
#  define WITH_POSTGRESQL
#endif  // BYCMAKE_WITH_POSTGRESQL

//  math defines
#define FLOAT_ACCURACY        0.00001
#define DOUBLE_ACCURACY       0.000000001

// status/state defines
typedef uint32_t mstatus_t;
/** \brief статус при инициализации */
#define STATUS_DEFAULT        0x00000001
/** \brief статус удачного результата операции */
#define STATUS_OK             0x00000002
/** \brief статус неудачного результата операции */
#define STATUS_NOT            0x00000003
/** \brief статус наличия ошибки при выполнении операции */
#define STATUS_HAVE_ERROR     0x00000004

// logging defines
#define DEFAULT_LOGLVL        0x01
#define DEBUG_LOGLVL          0x0f
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

#endif  // !_UTILS__COMMON_H_
