cmake_minimum_required(VERSION 3.5)
add_compile_options(-std=c++11 -Wall)

set (PROJECT_NAME test_models)
project(${PROJECT_NAME})

set(
  MAIN_APP_ROOT
  ${CMAKE_CURRENT_SOURCE_DIR}/../../..
)

set(
  MAIN_APP_CORELIBS_ROOT
  ${MAIN_APP_ROOT}/asp_therm/libs
)

set(
  SRC_MODELS_TEST
  test_models.cpp
  test_models_mix.cpp
)

INCLUDE_DIRECTORIES(
  ${MAIN_APP_ROOT}/asp_therm/source/core/subroutins
  ${MAIN_APP_ROOT}/asp_therm/source/core/common
  ${MAIN_APP_ROOT}/asp_therm/source/core/gas_parameters
  ${MAIN_APP_ROOT}/asp_therm/source/core/models
  ${MAIN_APP_ROOT}/asp_therm/include)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY
    ${MAIN_APP_ROOT}/build_test_asp_therm/models
)

link_directories(${MAIN_APP_CORELIBS_ROOT})
link_directories(${MAIN_APP_CORELIBS_ROOT}/debug)

add_executable(${PROJECT_NAME} ${SRC_MODELS_TEST})
target_link_libraries(${PROJECT_NAME} lmodels pugixml)
