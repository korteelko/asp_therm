/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#ifndef _GUI__PROGRAM_DATA_H_
#define _GUI__PROGRAM_DATA_H_

#include "ErrorWrap.h"

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
