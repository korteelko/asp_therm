/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#ifndef UTILS__LOGGING_H
#define UTILS__LOGGING_H

#include "common.h"
#include "ErrorWrap.h"
#include "ThreadWrap.h"

#include <fstream>
#include <string>
#include <sstream>

#ifdef OS_NIX
#  include <sys/param.h>
#elif defined(_OS_WIN)
//   don'checked
#  include <climits>
#endif  // OS_NIX

/* максимальный размер файла логов */
#define MAXSIZE_LOGFILE  128*1024  // 128 KiB

/** logging settings: level and filename */
struct logging_cfg {
  io_loglvl loglvl;
  char filepath[PATH_MAX];  //MAXPATHLEN

public:
  logging_cfg(io_loglvl ll, const std::string &file);
};

typedef std::ofstream mlog_fostream;

/* TODO может и эти дефайны переопределить в файле конфигурации */
/** \brief класс логирования сообщений */
class Logging {
public:
  /** \brief init class with default parameters **/
  static merror_t InitDefault();
  /** \brief change output file, log level **/
  static merror_t ResetInstance(const logging_cfg &li);
  static merror_t GetErrorCode();
  static io_loglvl GetLogLevel();
  static std::string GetStatusMessage();
  /** \brief force clear logfile */
  static void ClearLogfile();
  /** \brief append logfile with passed message
    *  if loglevel of instance != io_loglvl::no_log */
  // static void Append(const char *format, ...);
  static void Append(const std::string &msg);
  /** \brief append logfile with passed message
    * if loglevel of instance correspond to 'lvl'
    * else ignore */
  // static void Append(io_loglvl lvl, const char *format, ...);
  static void Append(io_loglvl lvl, const std::string &msg);
  /** \brief append logfile with passed 'msg'
    *   io_loglvl = err_logs */
  static void Append(merror_t error_code, const std::string &msg);
  /** \brief append logfile with passed 'msg' */
  static void Append(io_loglvl lvl, merror_t error_code,
      const std::string &msg);
  /** \brief append logfile with passed stringstream 'sstr' */
  static void Append(const std::stringstream &sstr);
  static void Append(io_loglvl lvl, const std::stringstream &sstr);
  // TODO: how about:
  // static void Append(ErrorWrap err, const std::string &msg);

private:
  static mlog_fostream output_;
  static logging_cfg li_;
  static ErrorWrap error_;
  static Mutex logfile_mutex_;
  // static RecursiveMutex append_mutex_;
  static bool is_aval_;
  static bool cerr_duplicate_;

private:
  /** \brief check logfile exist, check length of file */
  static merror_t checkInstance();
  /** \brief check instance and set variables */
  static merror_t initInstance(const logging_cfg *li);
  /** \brief append log */
  static void append(const char *msg);
  /** \brief reset logging configuration(filename, log level) */
  static void set_cfg(const logging_cfg *li);
};

#endif  // !UTILS__LOGGING_H
