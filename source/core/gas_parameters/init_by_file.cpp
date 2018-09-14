#include "init_by_file.h"

#include <assert.h>

GasParameters *InitGasParameters::Init(
    const std::string &filename) {
  assert(0);
  if (filename.empty()) {
    return nullptr;
  }
}

std::vector<GasParameters *> InitGasParameters::Init(
    const std::vector<std::string> *files) {
  assert(0);
  return std::vector<GasParameters *>();
}
