#ifndef _CORE__COMMON__MODELS_ERRORS_H_
#define _CORE__COMMON__MODELS_ERRORS_H_

#include <stdint.h>

/* swap uint32 to uint64 for simple padding
 *   merror_t ussualy first field of struct/class
 */
typedef uint64_t merror_t;

#define ERR_MSG_MAX_LEN    200

// error type
#define ERR_MASK_TYPE      0x000f

#define ERR_SUCCESS_T      0x0000
#define ERR_FILEIO_T       0x0001
#define ERR_CALCULATE_T    0x0002
#define ERR_STRING_T       0x0003
#define ERR_INIT_T         0x0004

// type gas / gas_mix
#define ERR_MASK_GAS_MIX   0x00f0
// посути просто маркер
//   смеси газов
// #define ERR_GAS_PURE     0x0000
// assert UDOLI
#define ERR_GAS_MIX        0x0010

// error subtype
#define ERR_MASK_SUBTYPE   0x0f00
// fileio errors
#define ERR_FILE_IN_ST     0x0100
#define ERR_FILE_OUT_ST    0x0200

// calculate errors
#define ERR_CALC_GAS_P_ST  0x0100
#define ERR_CALC_PHASE_ST  0x0200
#define ERR_CALC_MODEL_ST  0x0300
#define ERR_CALC_MIX_ST    0x0400

// string errors
#define ERR_STR_MAX_LEN_ST 0x0100
#define ERR_STR_PARSE_ST   0x0200
#define ERR_STR_NULL_ST    0x0300

// init errors
#define ERR_INIT_ZERO_ST   0x0100
#define ERR_INIT_NULLP_ST  0x0200

// not error, but
#define XML_LAST_STRING    0x1000


// ERR_SUCCESS as default
merror_t set_error_code(merror_t err);
merror_t get_error_code();
// set err_code to SUCCESS and
//   forget setted error message
void reset_error();

// replace custom errormsg with *msg
merror_t set_error_message(merror_t err_code, const char *msg);
// add to custom errormsg *msg
// TODO: replace 'add_to_err_msg' with function with variadic arg count:
//   merror_t set_error_message(merror_t err_code, const char *msg, ...);
void add_to_error_msg(const char *msg);
char *get_error_message();

#endif  // !_CORE__COMMON__MODELS_ERRORS_H_
