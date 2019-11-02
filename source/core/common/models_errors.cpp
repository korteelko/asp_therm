#include "models_errors.h"

#include <string.h>

static merror_t model_error = ERR_SUCCESS_T;
static char model_error_msg[ERR_MSG_MAX_LEN] = {0};

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
  merror_t err_type     = ERR_MASK_TYPE & model_error;
  merror_t err_concrete = ERR_MASK_SUBTYPE & model_error;
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

void set_error_code(merror_t err) {
  model_error = err;
}

merror_t get_error_code() {
  return model_error;
}

void reset_error() {
  model_error  = ERR_SUCCESS_T;
  *model_error_msg = '\0';
}

merror_t set_error_message(const char *msg) {
  if (!msg)
    return model_error;
  if (strlen(msg) > ERR_MSG_MAX_LEN) {
    strcpy(model_error_msg, "passed_errmsg too long. Print custom:\n  ");
    char *custom_err_msg = get_custom_err_msg();
    if (custom_err_msg != NULL)
      strcat(model_error_msg, custom_err_msg);
  } else {
    strcpy(model_error_msg, msg);
  }
  return model_error;
}

merror_t set_error_message(merror_t err_code, const char *msg) {
  set_error_message(msg);
  return model_error = err_code;
}

void add_to_error_msg(const char *msg) {
  char *custom_err_msg = get_custom_err_msg();
  if (custom_err_msg != NULL)
    strcpy(model_error_msg, custom_err_msg);
  if (strlen(model_error_msg) + strlen(msg) >= ERR_MSG_MAX_LEN)
    return;
  strcat(model_error_msg, msg);
}

char *get_error_message() {
  if (*model_error_msg != '\0')
    return model_error_msg;
  char *custom_err_msg = get_custom_err_msg();
  if (!custom_err_msg)
    return NULL;
  if (ERR_MASK_GAS_MIX & model_error) {
    strcpy(model_error_msg, "gasmix: ");
    strcat(model_error_msg, custom_err_msg);
    return model_error_msg;
  }
  return custom_err_msg;
}
