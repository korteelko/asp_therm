#include "models_logging.h"


#include <assert.h>

#define DEFAULT_LOGFILE  "models_log.txt"

mlog_fostream Logging::output_;
logging_cfg Logging::li_ = {
#ifdef _DEBUG
  debug_logs,
#else
  err_logs,
#endif  // _DEBUG
  DEFAULT_LOGFILE
};
merror_t Logging::error_ = ERR_SUCCESS_T;
bool Logging::is_aval_ = false;

asd

bool Logging::InitInstance() {

}

merror_t Logging::ResetInstance(logging_cfg &li) {

}

merror_t Logging::GetError() {

}

void Logging::ClearLogfile() {

}

void Logging::Append(const char &format, ...) {

}

void Logging::Append(io_loglvl lvl, const char &format, ...) {

}
