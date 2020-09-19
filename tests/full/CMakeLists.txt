cmake_minimum_required(VERSION 3.7)

set(MAIN_PROJECT "asp_therm")
project(${MAIN_PROJECT}-fulltests)

add_definitions(-DTESTING_PROJECT)

# so intresting and it's all about cmake and gtest!
#   https://cmake.org/cmake/help/latest/module/GoogleTest.html

set(ASP_THERM_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../")
set(ASP_THERM_CMAKE_ROOT ${ASP_THERM_ROOT}/cmake)
set(ASP_THERM_FULLTEST_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(MODULES_DIR ${ASP_THERM_ROOT}/lib)
# add_subdirectory("${ASP_THERM_ROOT}/lib/gtest" "lib/gtest")

find_package(GTest)
if(${GTEST_FOUND})
  include_directories(${GTEST_INCLUDE_DIRS})
  set(FULLTEST_LIBRARIES gtest gtest_main)

  # code coverage
  add_compile_options(-fprofile-arcs -ftest-coverage)
  list(APPEND FULLTEST_LIBRARIES gcov)

  include(${ASP_THERM_CMAKE_ROOT}/atherm_setup.cmake)

  find_package(Threads REQUIRED)
  list(APPEND FULLTEST_LIBRARIES Threads::Threads)

  set(THERMCORE_INCLUDES

    "${THERMCORE_SOURCE_DIR}/common"
    "${THERMCORE_SOURCE_DIR}/gas_parameters"
    "${THERMCORE_SOURCE_DIR}/models"
    "${THERMCORE_SOURCE_DIR}/phase_diagram"
    "${THERMCORE_SOURCE_DIR}/service"
    "${THERMCORE_SOURCE_DIR}/subroutins"
  )
  include_directories(
    ${THERMCORE_INCLUDES}
    ${THERMUTILS_SOURCE_DIR}
    ${THERMDB_SOURCE_DIR}
  )
  enable_testing()
  # core testes
  #   math
  include(${ASP_THERM_FULLTEST_DIR}/core/common/common_tests.cmake)
  #   gasmix
  include(${ASP_THERM_FULLTEST_DIR}/core/gas_parameters/gas_parameters_tests.cmake)
  #   state
  include(${ASP_THERM_FULLTEST_DIR}/core/service/state_tests.cmake)
  #   fileurl
  include(${ASP_THERM_FULLTEST_DIR}/utils/fileurl_tests.cmake)
  #   xml and json reader
  include(${ASP_THERM_FULLTEST_DIR}/utils/readers_tests.cmake)
  #   models tests
  include(${ASP_THERM_FULLTEST_DIR}/core/models/models_tests.cmake)
endif(${GTEST_FOUND})

