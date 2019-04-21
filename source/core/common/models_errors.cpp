#include "models_errors.h"

#include <string.h>

static ERROR_TYPE err_tmp     = ERR_SUCCESS_T;
static char err_msg[ERR_MSG_MAX_LEN] = {0};

static const char *custom_msg[] = {
  "there are not any errors",
  "fileio error",
  "calculation error",
  "string processing error",
  "init struct error"
};

static const char *custom_msg_fileio[] {
  "fileio error ",
  "input from file error ",
  "output to file error "
};

static const char *custom_msg_calculate[]= {
  "calculate error ",
  "parameters error ",
  "phase diagram error ",
  "model error ",
  "gas mix error"
};

static const char *custom_msg_string[] = {
  "string processing error ",
  "string len error ",
  "string parsing error ",
  "passed null string "
};

static const char *custom_msg_init[] = {
  "init error ",
  "zero value init ",
  "nullptr value init "
};

// Установим конкретное сообщение об ошибке из
//   приведенных выше
// set errmessage
static char *get_custom_err_msg() {
  ERROR_TYPE err_type     = ERR_MASK_TYPE & err_tmp;
  ERROR_TYPE err_concrete = ERR_MASK_SUBTYPE & err_tmp;
  // Прицеливаемся в ногу
  char **list_of_custom_msg = NULL;
  switch (err_type) {
    case ERR_SUCCESS_T:
      return (char *)custom_msg[ERR_SUCCESS_T];
    case ERR_FILEIO_T:
      list_of_custom_msg = (char **)custom_msg_fileio;
      break;
    case ERR_CALCULATE_T:
      list_of_custom_msg = (char **)custom_msg_calculate;
      break;
    case ERR_STRING_T:
      list_of_custom_msg = (char **)custom_msg_string;
      break;
    case ERR_INIT_T:
      list_of_custom_msg = (char **)custom_msg_init;
      break;
    default:
      break;
  }
  if (list_of_custom_msg != NULL)
    return list_of_custom_msg[err_concrete];
  return NULL;
}

void set_error_code(ERROR_TYPE err) {
  err_tmp = err;
}

ERROR_TYPE get_error_code() {
  return err_tmp;
}

void reset_error() {
  err_tmp  = ERR_SUCCESS_T;
  *err_msg = '\0';
}

void set_error_message(const char *msg) {
  if (!msg)
    return;
  if (strlen(msg) > ERR_MSG_MAX_LEN) {
    strcpy(err_msg, "passed_errmsg too long. Print custom:\n  ");
    char *custom_err_msg = get_custom_err_msg();
    if (custom_err_msg != NULL)
      strcat(err_msg, custom_err_msg);
    return;
  } else {
    strcpy(err_msg, msg);
  }
}

void set_error_message(int err_code, const char *msg) {
  set_error_code(err_code);
  set_error_message(msg);
}

void add_to_error_msg(const char *msg) {
  char *custom_err_msg = get_custom_err_msg();
  if (custom_err_msg != NULL)
    strcpy(err_msg, custom_err_msg);
  if (strlen(err_msg) + strlen(msg) >= ERR_MSG_MAX_LEN)
    return;
  strcat(err_msg, msg);
}

char *get_error_message() {
  if (*err_msg != '\0')
    return err_msg;
  char *custom_err_msg = get_custom_err_msg();
  if (!custom_err_msg)
    return NULL;
  if (ERR_MASK_GAS_MIX & err_tmp) {
    strcpy(err_msg, "gasmix: ");
    strcat(err_msg, custom_err_msg);
    return err_msg;
  }
  return custom_err_msg;
}
