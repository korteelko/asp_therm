/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#ifndef UTILS__MERROR_CODES_H
#define UTILS__MERROR_CODES_H


#if not defined(ERROR_SUCCESS_T)
#  define ERROR_SUCCESS_T       0x00000000
#endif  // !ERROR_SUCCESS_T
#if not defined(ERROR_GENERAL_T)
#  define ERROR_GENERAL_T       0x00000001
#endif  // !ERROR_GENERAL_T

// error type
#define ERROR_MASK_TYPE       0x000f
/** ошибка файлового ввода/вывода */
#define ERROR_FILEIO_T        0x0002
/** ошибка при проведении расчётов */
#define ERROR_CALCULATE_T     0x0003
/** ошибка при работе со строками */
#define ERROR_STRING_T        0x0004
/** ошибка инициализации */
#define ERROR_INIT_T          0x0005
/** ошибка работы со стоковыми шаблонами.
  *   строковые шаблоны - текстовые значения в xml и json файлах */
#define ERROR_STRTPL_T        0x0006
/** ошибка работы с базой данных */
#define ERROR_DATABASE_T      0x0007

// type gas / gas_mix
#define ERROR_MASK_GAS_MIX    0x00f0
// посути просто маркер
//   смеси газов
// #define ERROR_GAS_PURE      0x0000
// assert UDOLI
#define ERROR_GAS_MIX         0x0010

// error subtype
#define ERROR_MASK_SUBTYPE    0x0f00
// fileio errors
#define ERROR_FILE_IN_ST      (0x0100 | ERROR_FILEIO_T)
#define ERROR_FILE_OUT_ST     (0x0200 | ERROR_FILEIO_T)
#define ERROR_FILE_LOGGING_ST (0x0300 | ERROR_FILEIO_T)

// calculate errors
#define ERROR_CALC_GAS_P_ST   (0x0100 | ERROR_CALCULATE_T)
#define ERROR_CALC_PHASE_ST   (0x0200 | ERROR_CALCULATE_T)
#define ERROR_CALC_MODEL_ST   (0x0300 | ERROR_CALCULATE_T)
#define ERROR_CALC_MIX_ST     (0x0400 | ERROR_CALCULATE_T)

// string errors
#define ERROR_STR_MAX_LEN_ST  (0x0100 | ERROR_STRING_T)
#define ERROR_STR_PARSE_ST    (0x0200 | ERROR_STRING_T)
#define ERROR_STR_NULL_ST     (0x0300 | ERROR_STRING_T)
#define ERROR_STR_TOINT_ST    (0x0400 | ERROR_STRING_T)

// init errors
#define ERROR_INIT_ZERO_ST    (0x0100 | ERROR_INIT_T)
#define ERROR_INIT_NULLP_ST   (0x0200 | ERROR_INIT_T)

// string templates error
/** пустой текстовый шаблон */
#define ERROR_STRTPL_TPLNULL  (0x0100 | ERROR_STRTPL_T)
/** неизвестный текстовый шаблон */
#define ERROR_STRTPL_TPLUNDEF (0x0200 | ERROR_STRTPL_T)
/** недопустимое значение текстового шаблона */
#define ERROR_STRTPL_VALWRONG (0x0300 | ERROR_STRTPL_T)

// database connection
/** ошибка подключения к базе данных */
#define ERROR_DB_CONNECTION   (0x0100 | ERROR_DATABASE_T)
#define ERROR_DB_VARIABLE     (0x0200 | ERROR_DATABASE_T)
#define ERROR_DB_REFER_FIELD  (0x0300 | ERROR_DATABASE_T)


const char *GetCustomErrorMsg(unsigned int error);

#endif  // !UTILS__MERROR_CODES_H
