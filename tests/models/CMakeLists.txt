cmake_minimum_required(VERSION 3.5)
add_compile_options(-std=c++11 -Wall)

set (PROJECT_NAME test_models)
project(${PROJECT_NAME})

add_definitions(-D_DEBUG)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")

if(MSVC)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

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
  ${MAIN_APP_ROOT}/asp_therm/source/core/phase_diagram
  ${MAIN_APP_ROOT}/asp_therm/include
)

if (WIN32)
  INCLUDE_DIRECTORIES("${MAIN_APP_ROOT}/asp_therm/include/target_sys/win")
  link_directories(${MAIN_APP_CORELIBS_ROOT}/win/debug)
  link_directories(${MAIN_APP_CORELIBS_ROOT}/win)
#endif(WIN32)
else ()
#if(LINUX)
  INCLUDE_DIRECTORIES("${MAIN_APP_ROOT}/asp_therm/include/target_sys/_nix")
  link_directories(${MAIN_APP_CORELIBS_ROOT}/_nix/debug)
  link_directories(${MAIN_APP_CORELIBS_ROOT}/_nix)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY
    ${MAIN_APP_ROOT}/build_test_asp_therm/models
)


add_executable(${PROJECT_NAME} ${SRC_MODELS_TEST})
# 29.09.2018
# target_link_libraries(${PROJECT_NAME} lmodels pugixml)
target_link_libraries(${PROJECT_NAME} lmodels)
