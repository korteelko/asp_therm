#include <iostream>

#include "test_xml.h"

int main() {
  int failed_test = 0;
  failed_test += run_tests_xml();
  // **
  std::cout << "failed_test: " << failed_test << std::endl;
  return failed_test;
}
