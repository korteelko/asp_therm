#ifndef _UTILS__MERROR_CODES_H_
#define _UTILS__MERROR_CODES_H_


#if not defined(ERROR_SUCCESS_T)
#  define ERROR_SUCCESS_T   0x00000000
#endif  // !ERROR_SUCCESS_T
#if not defined(ERROR_GENERAL_T)
#  define ERROR_GENERAL_T   0x00000001
#endif  // !ERROR_GENERAL_T

// error type
#define ERR_MASK_TYPE       0x000f
/** ошибка файлового ввода/вывода */
#define ERR_FILEIO_T        0x0002
/** ошибка при проведении расчётов */
#define ERR_CALCULATE_T     0x0003
/** ошибка при работе со строками */
#define ERR_STRING_T        0x0004
/** ошибка инициализации */
#define ERR_INIT_T          0x0005
/** ошибка работы со стоковыми шаблонами.
  *   строковые шаблоны - текстовые значения в xml и json файлах */
#define ERR_STRTPL_T        0x0006
/** ошибка работы с базой данных */
#define ERR_DATABASE_T      0x0007

// type gas / gas_mix
#define ERR_MASK_GAS_MIX    0x00f0
// посути просто маркер
//   смеси газов
// #define ERR_GAS_PURE      0x0000
// assert UDOLI
#define ERR_GAS_MIX         0x0010

// error subtype
#define ERR_MASK_SUBTYPE    0x0f00
// fileio errors
#define ERR_FILE_IN_ST      (0x0100 | ERR_FILEIO_T)
#define ERR_FILE_OUT_ST     (0x0200 | ERR_FILEIO_T)
#define ERR_FILE_LOGGING_ST (0x0300 | ERR_FILEIO_T)

// calculate errors
#define ERR_CALC_GAS_P_ST   (0x0100 | ERR_CALCULATE_T)
#define ERR_CALC_PHASE_ST   (0x0200 | ERR_CALCULATE_T)
#define ERR_CALC_MODEL_ST   (0x0300 | ERR_CALCULATE_T)
#define ERR_CALC_MIX_ST     (0x0400 | ERR_CALCULATE_T)

// string errors
#define ERR_STR_MAX_LEN_ST  (0x0100 | ERR_STRING_T)
#define ERR_STR_PARSE_ST    (0x0200 | ERR_STRING_T)
#define ERR_STR_NULL_ST     (0x0300 | ERR_STRING_T)
#define ERR_STR_TOINT_ST    (0x0400 | ERR_STRING_T)

// init errors
#define ERR_INIT_ZERO_ST    (0x0100 | ERR_INIT_T)
#define ERR_INIT_NULLP_ST   (0x0200 | ERR_INIT_T)

// string templates error
/** пустой текстовый шаблон */
#define ERR_STRTPL_TPLNULL  (0x0100 | ERR_STRTPL_T)
/** неизвестный текстовый шаблон */
#define ERR_STRTPL_TPLUNDEF (0x0200 | ERR_STRTPL_T)
/** недопустимое значение текстового шаблона */
#define ERR_STRTPL_VALWRONG (0x0300 | ERR_STRTPL_T)

// database connection
/** ошибка подключения к базе данных */
#define ERR_DB_CONNECTION   (0x0100 | ERR_DATABASE_T)
#define ERR_DB_VARIABLE     (0x0200 | ERR_DATABASE_T)


const char *GetCustomErrorMsg(unsigned int error);

#endif  // !_UTILS__MERROR_CODES_H_
