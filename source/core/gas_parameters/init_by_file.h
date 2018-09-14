#ifndef _CORE__GAS_PARAMETERS__INIT_BY_FILE_H_
#define _CORE__GAS_PARAMETERS__INIT_BY_FILE_H_

#include "gas_description_static.h"

#include <string>
#include <vector>

class InitGasParameters {
  std::vector<std::string> content_;

public:
  static GasParameters *Init(const std::string &filename);
  static std::vector<GasParameters *>
      Init(const std::vector<std::string> *files);

};

#endif  // ! _CORE__GAS_PARAMETERS__INIT_BY_FILE_H_
