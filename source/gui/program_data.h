#ifndef _GUI__PROGRAM_DATA_H_
#define _GUI__PROGRAM_DATA_H_

#include "models_errors.h"

#include <string>
#include <vector>

class ProgramData {
  std::vector<std::string> gas_files_;
  std::vector<std::string> gas_names_;
  ERROR_TYPE error_status_;

private:
  ProgramData();
  bool ends_with(const char *src, const char *ending);
  void init_gas_files();
  void init_gas_names();
  int read_gases_dir(const std::string &dirname);

public:
  static ProgramData &Instance();
  ERROR_TYPE GetError();
  const std::vector<std::string> &GetGasesList();
};
#endif // !_GUI__PROGRAM_DATA_H_
