#include "merror_codes.h"

// ес, онгличанин ( ͡° ͜ʖ ͡°)
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

static const char *custom_msg_fileio[] {
  "fileio error ",
  "input from file error ",
  "output to file error "
  "error with logging file "
};

static const char *custom_msg_calculate[]= {
  "calculate error ",
  "parameters error ",
  "phase diagram error ",
  "model error ",
  "gas mix error "
};

static const char *custom_msg_string[] = {
  "string processing error ",
  "string len error ",
  "string parsing error ",
  "passed null string ",
  "convert to int "
};

static const char *custom_msg_init[] = {
  "init error ",
  "zero value init ",
  "nullptr value init "
};

static const char *custom_msg_strtpl[] = {
  "string template error ",
  "empty string template ",
  "undefined string template ",
  "wrong string template value "
};

static const char *custom_msg_database[] = {
  "database error ",
  "database connection error "
  "database variable error "
};

const char *GetCustomErrorMsg(unsigned int error) {
  unsigned int err_type     = ERROR_MASK_TYPE & error;
  unsigned int err_concrete = ERROR_MASK_SUBTYPE & error;
  // Прицеливаемся в ногу
  const char **list_of_custom_msg = nullptr;
  switch (err_type) {
    case ERROR_SUCCESS_T:
      return custom_msg[ERROR_SUCCESS_T];
    case ERROR_FILEIO_T:
      list_of_custom_msg = custom_msg_fileio;
      break;
    case ERROR_CALCULATE_T:
      list_of_custom_msg = custom_msg_calculate;
      break;
    case ERROR_STRING_T:
      list_of_custom_msg = custom_msg_string;
      break;
    case ERROR_INIT_T:
      list_of_custom_msg = custom_msg_init;
      break;
    case ERROR_STRTPL_T:
      list_of_custom_msg = custom_msg_strtpl;
      break;
    case ERROR_DATABASE_T:
      list_of_custom_msg = custom_msg_database;
      break;
    default:
      break;
  }
  return (list_of_custom_msg != nullptr) ?
      list_of_custom_msg[err_concrete] : nullptr;
}
