/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#include "common.h"

#include <algorithm>
#include <cctype>
#include <filesystem>

bool HasGostModelMark(const gas_marks_t m) {
  return m & GAS_NG_GOST_MARK;
}

bool HasGasMixMark(const gas_marks_t m) {
  return m & GAS_MIX_MARK;
}

void AddGostModelMark(gas_marks_t *m) {
  *m = *m | GAS_NG_GOST_MARK;
}

void AddGasMixMark(gas_marks_t *m) {
  *m = *m | GAS_MIX_MARK;
}

rg_model_id::rg_model_id(rg_model_t t, rg_model_subtype subt)
  : type(t), subtype(subt) {}

std::string trim_str(const std::string &str) {
  if (str.empty())
    return "";
  auto wsfront = std::find_if_not(str.begin(), str.end(),
      [](int c){return std::isspace(c);});
  return std::string(wsfront, std::find_if_not(str.rbegin(),
      std::string::const_reverse_iterator(wsfront),
      [](int c){return std::isspace(c);}).base());
}

bool is_exist(const std::string &path) {
  return std::filesystem::exists(path);
}

std::string dir_by_path(const std::string &path) {
  return std::filesystem::path(path).parent_path();
}

std::string hex2str(int hex) {
  std::stringstream hex_stream;
  hex_stream << hex;
  return hex_stream.str();
}

bool is_status_aval(mstatus_t status) {
  return status == STATUS_DEFAULT || status == STATUS_OK;
}
