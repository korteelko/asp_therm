#ifndef _CORE__COMMON__MODELS_LOGGING_H_
#define _CORE__COMMON__MODELS_LOGGING_H_

#include "models_errors.h"

#include <fstream>

#ifdef OS_NIX
#  include <sys/param.h>
#elif defined(_OS_WIN)
//   don'checked
#  include <climits>
#endif  // OS_NIX

#define DEFAULT_LOGLVL   0x01
#define DEBUG_LOGLVL     0x0f

typedef enum {
  no_log     = 0,              /* no messages     */
  err_logs   = DEFAULT_LOGLVL, /* only errors     */
  warn_logs,                   /* warning, errors */
  debug_logs = DEBUG_LOGLVL    /* all mesages, default for debug */
} io_loglvl;

/* logging settings: level, filename */
typedef struct {
  io_loglvl loglvl;
  char filepath[PATH_MAX];  //MAXPATHLEN
} logging_cfg;

typedef std::ofstream mlog_fostream;

class Logging {
  static mlog_fostream output_;
  static logging_cfg li_;
  static merror_t error_;
  static bool is_aval_;

private:
  static bool InitInstance();

public:
  /** change output file, log level **/
  static merror_t ResetInstance(logging_cfg &li);
  static merror_t GetError();
  static void ClearLogfile();
  static void Append(const char &format, ...);
  static void Append(io_loglvl lvl, const char &format, ...);
};

#endif  // !_CORE__COMMON__MODELS_LOGGING_H_
