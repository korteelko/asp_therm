#include "test_models_mix.h"
#include "model_redlich_kwong.h"

#include <iostream>

#include <assert.h>

namespace {
  const std::string xml_path = "/../../asp_therm/data/gases/";
  const std::string xml_methane = "methane.xml";
}  // unnammed namespace

int main() {
  int failed_tests = 0;
  failed_tests += test_models();
  failed_tests += test_models_mix();
  std::cerr << "failed tests: " << failed_tests;
  return failed_tests;
}
