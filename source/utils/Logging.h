#ifndef _CORE__COMMON__MODELS_LOGGING_H_
#define _CORE__COMMON__MODELS_LOGGING_H_

#include "common.h"
#include "ErrorWrap.h"

#include <fstream>

#ifdef OS_NIX
#  include <sys/param.h>
#elif defined(_OS_WIN)
//   don'checked
#  include <climits>
#endif  // OS_NIX

#define MAXSIZE_LOGFILE  128*1024  // 128 KiB

#define DEFAULT_LOGFILE  "models_logs"
#define OLD_LOGFILE_SFX  "_prev"

/** logging settings: level and filename */
typedef struct {
  io_loglvl loglvl;
  char filepath[PATH_MAX];  //MAXPATHLEN
} logging_cfg;

typedef std::ofstream mlog_fostream;

/* TODO может и эти дефайны переопределить в файле конфигурации */
/** \brief класс логирования сообщений */
class Logging {
  static mlog_fostream output_;
  static logging_cfg li_;
  static std::string status_msg_;
  static ErrorWrap error_;
  static bool is_aval_;

private:
  /** \brief check logfile exist, check length of file */
  static merror_t checkInstance();
  /** \brief check instance and set variables */
  static merror_t initInstance();
  /** \brief append log */
  static void append(const char *msg);

public:
  /** \brief init class with default parameters **/
  static merror_t InitDefault();
  /** \brief change output file, log level **/
  static merror_t ResetInstance(logging_cfg &li);
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

};

#endif  // !_CORE__COMMON__MODELS_LOGGING_H_
