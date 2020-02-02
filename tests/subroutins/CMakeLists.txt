cmake_minimum_required(VERSION 3.7)
add_compile_options(-std=c++17 -Wall)

set(PROJECT_NAME test_subroutins)
set(RESULT_LIB atherm)
project(${PROJECT_NAME})

add_definitions(-D_DEBUG)
add_definitions(-D_DEBUG_SUBROUTINS)
set(CMAKE_BUILD_TYPE Debug)

set(ASP_THERM_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../..")
set(THERMCORE_SOURCE_DIR "${ASP_THERM_ROOT}/source/core")

if(UNIX)
  add_definitions(-DOS_NIX)
  include_directories("${ASP_THERM_ROOT}/include/target_sys/_nix")
elseif(MSVC)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS -DOS_WIN)
  include_directories("${ASP_THERM_ROOT}/include/target_sys/win")
else()
  message(SEND_ERROR "UNDEFINED SYSTEM ${CMAKE_SYSTEM_NAME}")
endif(UNIX)

# LIBS_DIR - для моих библиотек
set(LIBS_DIR ${ASP_THERM_ROOT}/lib/${RESULT_LIB}/debug)
# MODULES_DIR - для сторонних библиотек.
set(MODULES_DIR ${ASP_THERM_ROOT}/lib)

# maybe split in separate asp_therm_modules.cmake ???
#   pugixml
set(PUGIXML_DIR "${MODULES_DIR}/pugixml")
include_directories(${PUGIXML_DIR}/src)
link_directories(${ASP_THERM_ROOT}/build/lib/pugixml)

set(
  SRC_SUBROUTINS_TEST

  subroutins_test_main.cpp
  test_xml.cpp

# common sources
  ${THERMCORE_SOURCE_DIR}/common/common.cpp
  ${THERMCORE_SOURCE_DIR}/common/models_errors.cpp
  ${THERMCORE_SOURCE_DIR}/common/models_logging.cpp
  ${THERMCORE_SOURCE_DIR}/common/models_math.cpp

# subroutins sources
  ${THERMCORE_SOURCE_DIR}/subroutins/file_structs.cpp
  ${THERMCORE_SOURCE_DIR}/subroutins/gas_by_file.cpp
  ${THERMCORE_SOURCE_DIR}/subroutins/gasmix_by_file.cpp

# gas_parameters sources
  ${THERMCORE_SOURCE_DIR}/gas_parameters/gas_description.cpp
  ${THERMCORE_SOURCE_DIR}/gas_parameters/gas_description_dynamic.cpp
  ${THERMCORE_SOURCE_DIR}/gas_parameters/gas_description_static.cpp
  ${THERMCORE_SOURCE_DIR}/gas_parameters/gasmix_init.cpp
  ${THERMCORE_SOURCE_DIR}/gas_parameters/gas_ng_gost_defines.cpp
)

set(THERMCORE_INCLUDES
  "${THERMCORE_SOURCE_DIR}/common"
  "${THERMCORE_SOURCE_DIR}/gas_parameters"
  "${THERMCORE_SOURCE_DIR}/models"
  "${THERMCORE_SOURCE_DIR}/phase_diagram"
  "${THERMCORE_SOURCE_DIR}/subroutins"
)
include_directories(${THERMCORE_INCLUDES})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY
    ${ASP_THERM_ROOT}/../build_test_asp_therm/subroutins
)

add_executable(${PROJECT_NAME} ${SRC_SUBROUTINS_TEST})
# target_link_libraries(${PROJECT_NAME} lmodels pugixml)
target_link_libraries(${PROJECT_NAME} pugixml)
