cmake_minimum_required(VERSION 3.5)
add_compile_options(-std=c++11 -Wall)

set (PROJECT_NAME test_models)
project(${PROJECT_NAME})

add_definitions(-D_DEBUG)
# i have problem with vscode
# add_definitions(-D_IDE_VSCODE)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")

if(MSVC)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

set(MAIN_APP_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../..)

set(THERMCORE_SOURCE_DIR "${MAIN_APP_ROOT}/asp_therm/source/core")

set(MAIN_APP_CORELIBS_ROOT ${MAIN_APP_ROOT}/asp_therm/libs)

set(
  SRC_MODELS_TEST
# common sources
  ${THERMCORE_SOURCE_DIR}/common/models_common.cpp
  ${THERMCORE_SOURCE_DIR}/common/models_errors.cpp
  ${THERMCORE_SOURCE_DIR}/common/models_math.cpp

# gas_parameters sources
  ${THERMCORE_SOURCE_DIR}/gas_parameters/gas_description.cpp
  ${THERMCORE_SOURCE_DIR}/gas_parameters/gas_description_dynamic.cpp
  ${THERMCORE_SOURCE_DIR}/gas_parameters/gas_description_static.cpp
  ${THERMCORE_SOURCE_DIR}/gas_parameters/gas_ng_gost_init.cpp
  ${THERMCORE_SOURCE_DIR}/gas_parameters/gas_ng_gost_defines.cpp
  ${THERMCORE_SOURCE_DIR}/gas_parameters/gasmix_init.cpp

# phase_diagram sources
  ${THERMCORE_SOURCE_DIR}/phase_diagram/phase_diagram.cpp
  ${THERMCORE_SOURCE_DIR}/phase_diagram/phase_diagram_models.cpp

# subroutins sources
  ${THERMCORE_SOURCE_DIR}/subroutins/gas_by_file.cpp
  ${THERMCORE_SOURCE_DIR}/subroutins/gasmix_by_file.cpp
  ${THERMCORE_SOURCE_DIR}/subroutins/xml_reader.cpp

# models sources
  ${THERMCORE_SOURCE_DIR}/models/model_general.cpp
  ${THERMCORE_SOURCE_DIR}/models/model_ideal_gas.cpp
  ${THERMCORE_SOURCE_DIR}/models/model_ng_gost.cpp
  ${THERMCORE_SOURCE_DIR}/models/model_peng_robinson.cpp
  ${THERMCORE_SOURCE_DIR}/models/model_redlich_kwong.cpp
  ${THERMCORE_SOURCE_DIR}/models/models_configurations.cpp
  ${THERMCORE_SOURCE_DIR}/models/models_creator.cpp

# test_src
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
  # link_directories(${MAIN_APP_CORELIBS_ROOT}/win/debug)
  link_directories(${MAIN_APP_CORELIBS_ROOT}/win)
#endif(WIN32)
else ()
#if(LINUX)
  INCLUDE_DIRECTORIES("${MAIN_APP_ROOT}/asp_therm/include/target_sys/_nix")
  # link_directories(${MAIN_APP_CORELIBS_ROOT}/_nix/debug)
  link_directories(${MAIN_APP_CORELIBS_ROOT}/_nix)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY
    ${MAIN_APP_ROOT}/build_test_asp_therm/models
)


add_executable(${PROJECT_NAME} ${SRC_MODELS_TEST})
# 29.09.2018
# target_link_libraries(${PROJECT_NAME} lmodels pugixml)
target_link_libraries(${PROJECT_NAME} pugixml)
