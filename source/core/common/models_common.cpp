#include "models_common.h"

#include <algorithm>
#include <cctype>

std::string F_was_pressed() {
  return "Pay respect to:\n"
      "\t--Stack Overflow community\n"
      "\t--Google\n"
      "\t--The Linux Foundation\n";
}

std::string trim_str(const std::string &str) {
  if (str.empty())
    return "";
  auto wsfront = std::find_if_not(str.begin(), str.end(),
      [](int c){return std::isspace(c);});
  return std::string(wsfront, std::find_if_not(str.rbegin(),
      std::string::const_reverse_iterator(wsfront),
      [](int c){return std::isspace(c);}).base());
}
