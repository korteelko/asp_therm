cmake_minimum_required(VERSION 3.7)

project(asp_therm-fulltests)

# so intresting and it's all about cmake and gtest!
#   https://cmake.org/cmake/help/latest/module/GoogleTest.html

include(gtest_setup.cmake)
set(ASP_THERM_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../)

if(${GTEST_FOUND})
  include(${ASP_THERM_ROOT}/atherm_setup.cmake)

  find_package(Threads REQUIRED)

  set(ASP_THERM_FULLTEST_DIR ${CMAKE_CURRENT_SOURCE_DIR})
  set(THERMCORE_INCLUDES
    "${THERMCORE_SOURCE_DIR}/common"
    "${THERMCORE_SOURCE_DIR}/gas_parameters"
    "${THERMCORE_SOURCE_DIR}/models"
    "${THERMCORE_SOURCE_DIR}/phase_diagram"
    "${THERMCORE_SOURCE_DIR}/subroutins"
  )
  include_directories(
    ${THERMCORE_INCLUDES}
    ${THERMUTILS_SOURCE_DIR}
    ${THERMDB_SOURCE_DIR}

    ${GTEST_INCLUDE_DIRS}
  )
  enable_testing()
  # core testes
  #   math
  include(${ASP_THERM_FULLTEST_DIR}/core/common/common_tests.cmake)
  #   gasmix
  include(${ASP_THERM_FULLTEST_DIR}/core/gas_parameters/gas_parameters_tests.cmake)
endif(${GTEST_FOUND})
