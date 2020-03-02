/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#include "ErrorWrap.h"

#include "Logging.h"


ErrorWrap::ErrorWrap()
  : ErrorWrap(ERROR_SUCCESS_T, "") {}

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
  if (error_ != ERROR_SUCCESS_T && !is_logged_) {
    if (!msg_.empty()) {
      Logging::Append(lvl, "Error occurred.\n  err_msg:" + msg_ +
          "\n  code:0x" + hex2str(error_));
  #if defined(INCLUDE_ERRORCODES)
    } else {
      Logging::Append(lvl, "Error occurred.\n  custom err_msg:" +
          std::string(GetCustomErrorMsg(error_)) +
          "\n  code:0x" + hex2str(error_));
  #endif  // INCLUDE_ERRORCODES
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

void ErrorWrap::Reset() {
  error_ = ERROR_SUCCESS_T;
  msg_ = "";
  is_logged_ = false;
}
