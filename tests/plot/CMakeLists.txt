cmake_minimum_required(VERSION 3.5)
add_compile_options(-std=c++11 -Wall)

set (PROJECT_NAME test_plot)
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
  SRC_PLOT_TEST
  test_plot.cpp
  # вот его надо реализовать
  # ${MAIN_APP_ROOT}/asp_therm/source/plot/plot_main.cpp
)

INCLUDE_DIRECTORIES(
  # ${MAIN_APP_ROOT}/asp_therm/source/core/common
  ${MAIN_APP_ROOT}/asp_therm/include)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY
    ${MAIN_APP_ROOT}/build_test_asp_therm/plot
)

link_directories(${MAIN_APP_CORELIBS_ROOT})
link_directories(${MAIN_APP_CORELIBS_ROOT}/debug)

add_executable(${PROJECT_NAME} ${SRC_PLOT_TEST})
