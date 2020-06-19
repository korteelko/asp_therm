/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#include "merror_codes.h"

// ес, онгличанин ( ͡° ͜ʖ ͡°)
// ! check comma after last but one message
static const char *custom_msg[] = {
  "there are not any errors ",
  /* not used messages(first message
   *   in concrete list of custom messages) */
  "default error ",
  "fileio error ",
  "calculation error ",
  "string processing error ",
  "init struct error ",
  "string template error "
};
#define custom_msg_size \ sizeof(custom_msg) / sizeof(*custom_msg)

static const char *custom_msg_fileio[] {
  "fileio error ",
  "input from file error ",
  "output to file error ",
  "error with logging file ",
  "parse json error ",
  "json format error ",
  "error child node search "
};
#define custom_msg_fileio_size \
    sizeof(custom_msg_fileio) / sizeof(*custom_msg_fileio)

static const char *custom_msg_calculate[]= {
  "calculate error ",
  "parameters error ",
  "phase diagram error ",
  "model error ",
  "gas mix error "
};
#define custom_msg_calculate_size \
    sizeof(custom_msg_calculate) / sizeof(*custom_msg_calculate)

static const char *custom_msg_string[] = {
  "string processing error ",
  "string len error ",
  "string parsing error ",
  "passed null string ",
  "convert to int "
};
#define custom_msg_string_size \
    sizeof(custom_msg_string) / sizeof(*custom_msg_string)

static const char *custom_msg_init[] = {
  "init error ",
  "zero value init ",
  "nullptr value init "
};
#define custom_msg_init_size \
    sizeof(custom_msg_init) / sizeof(*custom_msg_init)

static const char *custom_msg_strtpl[] = {
  "string template error ",
  "empty string template ",
  "undefined string template ",
  "wrong string template value "
};
#define custom_msg_strtpl_size \
    sizeof(custom_msg_strtpl) / sizeof(*custom_msg_strtpl)

static const char *custom_msg_database[] = {
  "database error ",
  "database connection error ",
  "database variable error ",
  "database reference to another table error ",
  "database table exist error ",
  "database query setup - database nullptr error ",
  "database table primary key setup error ",
  "database sql query exception ",
  "database wrong operation name ",
  "database table column doesn't exists ",
  "database save point error ",
};
#define custom_msg_database_size \
    sizeof(custom_msg_database) / sizeof(*custom_msg_database)

const char *GetCustomErrorMsg(unsigned int error) {
  /* специализация */
  unsigned int err_type = ERROR_MASK_TYPE & error;
  unsigned int err_concrete = (ERROR_MASK_SUBTYPE & error) >> 8;
  unsigned int array_size = 0;
  const char **list_of_custom_msg = nullptr;
  switch (err_type) {
    case ERROR_SUCCESS_T:
      return custom_msg[ERROR_SUCCESS_T];
    case ERROR_GENERAL_T:
      return custom_msg[ERROR_GENERAL_T];
    case ERROR_FILEIO_T:
      list_of_custom_msg = custom_msg_fileio;
      array_size = custom_msg_fileio_size;
      break;
    case ERROR_CALCULATE_T:
      list_of_custom_msg = custom_msg_calculate;
      array_size = custom_msg_calculate_size;
      break;
    case ERROR_STRING_T:
      list_of_custom_msg = custom_msg_string;
      array_size = custom_msg_string_size;
      break;
    case ERROR_INIT_T:
      list_of_custom_msg = custom_msg_init;
      array_size = custom_msg_init_size;
      break;
    case ERROR_STRTPL_T:
      list_of_custom_msg = custom_msg_strtpl;
      array_size = custom_msg_strtpl_size;
      break;
    case ERROR_DATABASE_T:
      list_of_custom_msg = custom_msg_database;
      array_size = custom_msg_database_size;
      break;
    default:
      break;
  }
  if (list_of_custom_msg != nullptr) {
    return (array_size > err_concrete) ?
      list_of_custom_msg[err_concrete] : nullptr;
  }
  return nullptr;
}
