cmake_minimum_required(VERSION 3.5)
add_compile_options(-std=c++11 -Wall)

set (PROJECT_NAME test_subroutins)
project(${PROJECT_NAME})

add_definitions(-D_DEBUG)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")

set(
  MAIN_APP_ROOT
  ${CMAKE_CURRENT_SOURCE_DIR}/../../..
)

if(WIN32)
  set(
    MAIN_APP_CORELIBS_ROOT
    ${MAIN_APP_ROOT}/asp_therm/libs/win
  )
elseif(UNIX)
  set(
    MAIN_APP_CORELIBS_ROOT
    ${MAIN_APP_ROOT}/asp_therm/libs/_nix
  )
endif()
link_directories(${MAIN_APP_CORELIBS_ROOT})

set(
  SRC_SUBROUTINS_TEST
  subroutins_test_main.cpp
  test_xml.cpp
  ${MAIN_APP_ROOT}/asp_therm/source/core/subroutins/xml_reader.cpp
  ${MAIN_APP_ROOT}/asp_therm/source/core/subroutins/gas_by_file.cpp
  ${MAIN_APP_ROOT}/asp_therm/source/core/subroutins/gasmix_by_file.cpp
  ${MAIN_APP_ROOT}/asp_therm/source/core/common/models_errors.cpp
  ${MAIN_APP_ROOT}/asp_therm/source/core/common/models_math.cpp
  
# gas_parameters sources
  ${MAIN_APP_ROOT}/asp_therm/source/core/gas_parameters/gas_description.cpp
  ${MAIN_APP_ROOT}/asp_therm/source/core/gas_parameters/gas_description_dynamic.cpp
  ${MAIN_APP_ROOT}/asp_therm/source/core/gas_parameters/gas_description_static.cpp
  ${MAIN_APP_ROOT}/asp_therm/source/core/gas_parameters/gasmix_init.cpp
  ${MAIN_APP_ROOT}/asp_therm/source/core/gas_parameters/gas_ng_gost_defines.cpp
)

INCLUDE_DIRECTORIES(
  ${MAIN_APP_ROOT}/asp_therm/source/core/subroutins
  ${MAIN_APP_ROOT}/asp_therm/source/core/common
  ${MAIN_APP_ROOT}/asp_therm/source/core/gas_parameters
  ${MAIN_APP_ROOT}/asp_therm/source/core/models
  ${MAIN_APP_ROOT}/asp_therm/source/core/phase_diagram
  ${MAIN_APP_ROOT}/asp_therm/include
)
if(WIN32)
  INCLUDE_DIRECTORIES(${MAIN_APP_ROOT}/asp_therm/include/target_sys/win)
elseif(UNIX)
  INCLUDE_DIRECTORIES(${MAIN_APP_ROOT}/asp_therm/include/target_sys/_nix)
endif()

#include_directories(
#  ${MAIN_APP_ROOT}/asp_therm/source/include)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY
    ${MAIN_APP_ROOT}/build_test_asp_therm/subroutins
)

# link_directories(${MAIN_APP_CORELIBS_ROOT})
# link_directories(${MAIN_APP_CORELIBS_ROOT}/debug)

add_executable(${PROJECT_NAME} ${SRC_SUBROUTINS_TEST})
# target_link_libraries(${PROJECT_NAME} lmodels pugixml)
target_link_libraries(${PROJECT_NAME} pugixml)
