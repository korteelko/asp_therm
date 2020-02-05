#include "models_logging.h"

#include <ctime>
#include <filesystem>

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

mlog_fostream Logging::output_;
logging_cfg Logging::li_ = {
#ifdef _DEBUG
  debug_logs,
#else
  err_logs,
#endif  // _DEBUG
  DEFAULT_LOGFILE
};
std::string Logging::status_msg_ = "";
merror_t Logging::error_ = ERR_SUCCESS_T;
bool Logging::is_aval_ = false;

namespace  {
// it's not cool remove this to new class of programm state
merror_t definit_err = Logging::InitDefault();
}  // namespace

merror_t Logging::checkInstance() {
  // don't try call models_errors.h function
  //   in this function)
  merror_t error = ERR_FILEIO_T;
  Logging::status_msg_ = "";
  if (*Logging::li_.filepath != '\0') {
    if (Logging::output_.is_open()) {
  # if !defined(NDEBUG)
      assert(0 && "logging module error");
  # else
      Logging::status_msg_ = "error in progger DNA";
      Logging::output_.close();
  #endif
    }
    Logging::output_.open(Logging::li_.filepath,
        std::fstream::out | std::fstream::app);
    // if file open check size, if size > MAXSIZE_LOGFILE
    //   move logging file to 'logging file_old'
    if (Logging::output_.is_open()) {
      Logging::output_.seekp(0, Logging::output_.end);
      auto logfile_size = Logging::output_.tellp();
      // Logging::output_.seekp(0, Logging::output_.beg);
      Logging::output_.close();
      if (logfile_size > MAXSIZE_LOGFILE) {
        std::string logfile_copy(Logging::li_.filepath);
        logfile_copy += OLD_LOGFILE_SFX;
        std::filesystem::rename(
            Logging::li_.filepath, logfile_copy);
      }
      error = ERR_SUCCESS_T;
    } else {
      error = ERR_FILEIO_T | ERR_FILE_OUT_ST;
      Logging::status_msg_ = "cannot open logging file";
    }
  }
  return error;
}

merror_t Logging::initInstance() {
  Logging::error_ = checkInstance();
  if (Logging::error_) {
    is_aval_ = false;
  } else {
    is_aval_ = true;
    // print start sequence
    Logging::output_.open(Logging::li_.filepath,
        std::fstream::out | std::fstream::app);
    if (Logging::output_.is_open()) {
      std::time_t tm = std::time(nullptr);
      Logging::output_ << "\n\n"
          << "==========================================================\n"
          << "Logging by " << std::asctime(std::localtime(&tm)) << "\n"
          << "==========================================================";
      if (!Logging::status_msg_.empty())
        Logging::output_ << "!log loga:) " << Logging::status_msg_;
      Logging::output_ << "\n";
      Logging::output_.close();
    } else {
      is_aval_ = false;
      Logging::error_ = ERR_FILEIO_T | ERR_FILE_LOGGING;
    }
  }
  if (Logging::error_)
    set_error_message(Logging::error_, "Open loggingfile end with error.\n"
        "  Message: %s\n", Logging::status_msg_.c_str());
  return Logging::error_;
}

void Logging::append(const char *msg) {
  Logging::output_.open(Logging::li_.filepath,
      std::fstream::out | std::fstream::app);
  if (Logging::output_.is_open()) {
    if (msg) {
      Logging::output_ << msg;
      if (!strrchr(msg, '\n'))
        Logging::output_.put('\n');
    }
    Logging::output_.close();
  } else {
    Logging::is_aval_ = false;
    Logging::error_ = ERR_FILEIO_T | ERR_FILE_LOGGING;
  }
}

merror_t Logging::InitDefault() {
  return initInstance();
}

merror_t Logging::ResetInstance(logging_cfg &li) {
  Logging::li_.loglvl = li.loglvl;
  memset(Logging::li_.filepath, 0, sizeof(Logging::li_.filepath));
  strncpy(Logging::li_.filepath, li.filepath, sizeof(Logging::li_.filepath));
  return initInstance();
}

merror_t Logging::GetError() {
  return Logging::error_;
}

io_loglvl Logging::GetLogLevel() {
  return Logging::li_.loglvl;
}

void Logging::ClearLogfile() {
  Logging::output_.open(Logging::li_.filepath, std::fstream::out);
  if (Logging::output_.is_open()) {
    Logging::output_.close();
  } else {
    Logging::is_aval_ = false;
    Logging::error_ = ERR_FILEIO_T | ERR_FILE_LOGGING;
  }
  return;
}

void Logging::Append(const std::string &msg) {
  if (Logging::is_aval_)
    if (!msg.empty() && (Logging::li_.loglvl != io_loglvl::no_log))
      Logging::append(msg.c_str());
}

void Logging::Append(io_loglvl lvl, const std::string &msg) {
  if (Logging::is_aval_)
    if (lvl <= Logging::li_.loglvl)
      if (!msg.empty() && (Logging::li_.loglvl != io_loglvl::no_log))
        Logging::append(msg.c_str());
}
