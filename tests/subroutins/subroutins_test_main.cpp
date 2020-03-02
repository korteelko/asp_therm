/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#include <iostream>

#include "test_xml.h"

int main() {
  int failed_test = 0;
  failed_test += run_tests_xml();
  // **
  std::cout << "failed_test: " << failed_test << std::endl;
  return failed_test;
}
