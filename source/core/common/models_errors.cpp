#include "models_errors.h"

#include "models_logging.h"

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static merror_t models_error = ERR_SUCCESS_T;
static char models_error_msg[ERR_MSG_MAX_LEN] = {0};

// ес, онгличанин ( ͡° ͜ʖ ͡°)
static const char *custom_msg[] = {
  "there are not any errors ",
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
  "init error ",
  "zero value init ",
  "nullptr value init "
};

// Установим конкретное сообщение об ошибке из
//   приведенных выше
// set errmessage
static const char *get_custom_err_msg(merror_t error) {
  merror_t err_type     = ERR_MASK_TYPE & error;
  merror_t err_concrete = ERR_MASK_SUBTYPE & error;
  // Прицеливаемся в ногу
  const char **list_of_custom_msg = nullptr;
  switch (err_type) {
    case ERR_SUCCESS_T:
      return custom_msg[ERR_SUCCESS_T];
    case ERR_FILEIO_T:
      list_of_custom_msg = custom_msg_fileio;
      break;
    case ERR_CALCULATE_T:
      list_of_custom_msg = custom_msg_calculate;
      break;
    case ERR_STRING_T:
      list_of_custom_msg = custom_msg_string;
      break;
    case ERR_INIT_T:
      list_of_custom_msg = custom_msg_init;
      break;
    case ERR_STRTPL_T:
      list_of_custom_msg = custom_msg_strtpl;
      break;
    default:
      break;
  }
  return (list_of_custom_msg != nullptr) ?
      list_of_custom_msg[err_concrete] : nullptr;
}

static const char *get_custom_err_msg() {
  return get_custom_err_msg(models_error);
}

merror_t set_error_code(merror_t err_code) {
  /* drop previous msg */
  *models_error_msg = '\0';
  Logging::Append(io_loglvl::err_logs,
      "Error occurred! Error code: 0x%h,\nCustom err_msg:\n  %s",
       err_code, get_custom_err_msg());
  return models_error = err_code;
}

merror_t get_error_code() {
  return models_error;
}

void reset_error() {
  models_error  = ERR_SUCCESS_T;
  *models_error_msg = '\0';
}

merror_t set_error_message(merror_t err_code,
    const char *format, ...) {
  if (!format)
    return set_error_code(err_code);
  memset(models_error_msg, 0, ERR_MSG_MAX_LEN);
  va_list args;
  va_start(args, format);
  /* all correct, snprintf copy n-1 character,
   *   saving last '\0' symbol */
  snprintf(models_error_msg, ERR_MSG_MAX_LEN, format, args);
  va_end(args);
  Logging::Append(io_loglvl::err_logs,
      "Error occurred! Error message:\n  %s", models_error_msg);
  return models_error = err_code;
}

const char *get_error_message() {
  const char *msg_ptr = models_error_msg;
  if (*msg_ptr == '\0') {
    msg_ptr = get_custom_err_msg();
    if (msg_ptr && (ERR_MASK_GAS_MIX & models_error)) {
      strcpy(models_error_msg, "gasmix: ");
      strcat(models_error_msg, msg_ptr);
      msg_ptr = models_error_msg;
    }
  }
  return msg_ptr;
}

ErrorWrap::ErrorWrap()
  : ErrorWrap(ERR_SUCCESS_T, "") {}

ErrorWrap::ErrorWrap(merror_t error)
  : ErrorWrap(error, "") {}

ErrorWrap::ErrorWrap(merror_t error, const std::string &msg)
  : error_(error), msg_(msg), is_logged_(false) {}

merror_t ErrorWrap::SetError(merror_t error) {
  return error_ = error;
}

merror_t ErrorWrap::SetError(merror_t error, const std::string &msg) {
  msg_ = msg;
  return error_ = error;
}

void ErrorWrap::ChangeMessage(const std::string &new_msg) {
  msg_ = new_msg;
}

void ErrorWrap::LogIt(io_loglvl lvl) {
  if (error_ != ERR_SUCCESS_T && !is_logged_) {
    if (!msg_.empty()) {
      Logging::Append(lvl, "Error occurred.\n  err_msg:%s\n  code:0x%h",
          msg_.c_str(), error_);
    } else {
      Logging::Append(lvl, "Error occurred.\n  custom err_msg:%s\n  code:0x%h",
          get_custom_err_msg(error_), error_);
    }
    is_logged_ = true;
  }
}

void ErrorWrap::LogIt() {
  LogIt(Logging::GetLogLevel());
}

merror_t ErrorWrap::GetErrorCode() const {
  return error_;
}

std::string ErrorWrap::GetMessage() const {
  return msg_;
}

bool ErrorWrap::IsLogged() const {
  return is_logged_;
}
