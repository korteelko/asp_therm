/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020-2021 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#ifndef UTILS__MERROR_CODES_H
#define UTILS__MERROR_CODES_H


#if not defined(ERROR_SUCCESS_T)
#  define ERROR_SUCCESS_T         0x0000
#  define ERROR_SUCCESS_T_MSG     "there are not any errors "
#endif  // !ERROR_SUCCESS_T
#if not defined(ERROR_GENERAL_T)
#  define ERROR_GENERAL_T         0x0001
#  define ERROR_GENERAL_T_MSG     "general error "
#endif  // !ERROR_GENERAL_T

// error type
#define ERROR_MASK_TYPE           0x00ff
/** \brief ошибка при проведении расчётов */
#define ERROR_CALCULATE_T         0x0003
#define ERROR_CALCULATE_T_MSG     "calculation error "
/** \brief ошибка работы со стоковыми шаблонами.
  *   строковые шаблоны - текстовые значения в xml и json файлах */
#define ERROR_STRTPL_T            0x0006
#define ERROR_STRTPL_T_MSG        "string template error "

// error subtype
#define ERROR_MASK_SUBTYPE        0xff00
//   fileio errors

//   calculate errors
#define ERROR_CALC_GAS_P_ST       (0x0100 | ERROR_CALCULATE_T)
#define ERROR_CALC_GAS_P_ST_MSG   "parameters error "
#define ERROR_CALC_PHASE_ST       (0x0200 | ERROR_CALCULATE_T)
#define ERROR_CALC_PHASE_ST_MSG   "phase diagram error "
#define ERROR_CALC_MODEL_ST       (0x0300 | ERROR_CALCULATE_T)
#define ERROR_CALC_MODEL_ST_MSG   "model error "
#define ERROR_CALC_MIX_ST         (0x0400 | ERROR_CALCULATE_T)
#define ERROR_CALC_MIX_ST_MSG     "gas mix error "

// string templates error
/** \brief пустой текстовый шаблон */
#define ERROR_STRTPL_TPLNULL      (0x0100 | ERROR_STRTPL_T)
#define ERROR_STRTPL_TPLNULL_MSG  "empty string template "
/** \brief неизвестный текстовый шаблон */
#define ERROR_STRTPL_TPLUNDEF     (0x0200 | ERROR_STRTPL_T)
#define ERROR_STRTPL_TPLUNDEF_MSG "undefined string template "
/** \brief недопустимое значение текстового шаблона */
#define ERROR_STRTPL_VALWRONG     (0x0300 | ERROR_STRTPL_T)
#define ERROR_STRTPL_VALWRONG_MSG "wrong string template value "

#endif  // !UTILS__MERROR_CODES_H
