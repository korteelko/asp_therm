cmake_minimum_required(VERSION 3.7)

set(PROJECT_NAME asp_therm)
set(RESULT_LIB atherm)
project(${PROJECT_NAME})
# TODO:
#   CHECK WINDOWS BUILDING

add_compile_options(-Wall)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# set cmake options:
# debug
add_definitions(-D_DEBUG)
set(CMAKE_BUILD_TYPE Debug)
#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
# release
# add_definitions(-DNDEBUG)
# set(CMAKE_BUILD_TYPE "Release")
# set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall")

# setup cmake directories
set(ASP_THERM_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(THERMCORE_SOURCE_DIR "${ASP_THERM_ROOT}/source/core")
set(THERMDB_SOURCE_DIR "${ASP_THERM_ROOT}/source/database")
set(THERMUTILS_SOURCE_DIR "${ASP_THERM_ROOT}/source/utils")

# setup cmake variables
set(DEFINES_SOURCE "")
set(ERRORCODES_FILE "merror_codes.h")
set(ERRORCODES_PATH "${THERMCORE_SOURCE_DIR}/common/${ERRORCODES_FILE}")
if(EXISTS ${ERRORCODES_PATH})
  message(STATUS "Add file with codes of errors: ${ERRORCODES_PATH}")
  # include(${ERRORCODES_PATH})
  set(INCLUDE_ERRORCODES TRUE)
  add_definitions(-DINCLUDE_ERRORCODES)
endif()


if(UNIX)
  add_definitions(-DOS_NIX)
  include_directories("include/target_sys/_nix")
elseif(MSVC)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS -DOS_WIN)
  include_directories("include/target_sys/win")
else()
  message(SEND_ERROR "UNDEFINED SYSTEM ${CMAKE_SYSTEM_NAME}")
endif(UNIX)

# setup utils


# LIBS_DIR - для моих библиотек
set(LIBS_DIR ${ASP_THERM_ROOT}/lib/${RESULT_LIB})
# MODULES_DIR - для сторонних библиотек.
set(MODULES_DIR ${ASP_THERM_ROOT}/lib)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(LIBS_DIR "${LIBS_DIR}/debug")
endif()
link_directories(${LIBS_DIR})


# maybe split in separate asp_therm_modules.cmake ???
# modules
#   pugixml
set(PUGIXML_DIR "${MODULES_DIR}/pugixml")
add_subdirectory(${PUGIXML_DIR})
include_directories(${PUGIXML_DIR}/src)

#   rapidjson
set(RAPIDJSON_DIR "${MODULES_DIR}/rapidjson")
add_subdirectory(${RAPIDJSON_DIR})
include_directories(${RAPIDJSON_DIR}/include)

message(STATUS "asp_therm cmake build info:")
message(STATUS "\t\tSystem info: ${CMAKE_SYSTEM_NAME}")
message(STATUS "\t\tBUILD_TYPE: ${CMAKE_BUILD_TYPE}")
message(STATUS "\t\tLIBS_DIR: ${LIBS_DIR}")

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
)

set(
  MODELS_SRC

# common sources
  ${THERMCORE_SOURCE_DIR}/common/common.cpp
  ${THERMCORE_SOURCE_DIR}/common/models_math.cpp

# gas_parameters sources
  ${THERMCORE_SOURCE_DIR}/gas_parameters/gas_description.cpp
  ${THERMCORE_SOURCE_DIR}/gas_parameters/gas_description_dynamic.cpp
  ${THERMCORE_SOURCE_DIR}/gas_parameters/gas_description_static.cpp
  ${THERMCORE_SOURCE_DIR}/gas_parameters/gas_ng_gost.cpp
  ${THERMCORE_SOURCE_DIR}/gas_parameters/gas_ng_gost_defines.cpp
  ${THERMCORE_SOURCE_DIR}/gas_parameters/gasmix_init.cpp

# phase_diagram sources
  ${THERMCORE_SOURCE_DIR}/phase_diagram/phase_diagram.cpp
  ${THERMCORE_SOURCE_DIR}/phase_diagram/phase_diagram_models.cpp

# subroutins sources
  ${THERMCORE_SOURCE_DIR}/subroutins/configuration_by_file.cpp
  ${THERMCORE_SOURCE_DIR}/subroutins/gas_by_file.cpp
  ${THERMCORE_SOURCE_DIR}/subroutins/gasmix_by_file.cpp
  ${THERMCORE_SOURCE_DIR}/subroutins/file_structs.cpp

# models sources
  ${THERMCORE_SOURCE_DIR}/models/model_general.cpp
  ${THERMCORE_SOURCE_DIR}/models/model_ideal_gas.cpp
  ${THERMCORE_SOURCE_DIR}/models/model_ng_gost.cpp
  ${THERMCORE_SOURCE_DIR}/models/model_peng_robinson.cpp
  ${THERMCORE_SOURCE_DIR}/models/model_redlich_kwong.cpp
  # ${THERMCORE_SOURCE_DIR}/models/model_redlich_kwong_soave.cpp
  ${THERMCORE_SOURCE_DIR}/models/models_configurations.cpp
  ${THERMCORE_SOURCE_DIR}/models/models_creator.cpp


# utils sources
  ${THERMUTILS_SOURCE_DIR}/ErrorWrap.cpp
  ${THERMUTILS_SOURCE_DIR}/Logging.cpp

# database sources
  ${THERMDB_SOURCE_DIR}/db_connection.cpp
  ${THERMDB_SOURCE_DIR}/db_connection_postgre.cpp
  ${THERMDB_SOURCE_DIR}/db_connection_manager.cpp
  ${THERMDB_SOURCE_DIR}/db_defines.cpp
  ${THERMDB_SOURCE_DIR}/db_query.cpp

# plot sources
# maybe OpenGL libs
)
if(INCLUDE_ERRORCODES)
  LIST(APPEND MODELS_SRC
      "${THERMCORE_SOURCE_DIR}/common/merror_codes.cpp")
endif()

# output for static libraries
add_library(${RESULT_LIB} STATIC ${MODELS_SRC})
set_target_properties(${RESULT_LIB} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${LIBS_DIR}
    ARCHIVE_OUTPUT_DIRECTORY ${LIBS_DIR}
)

foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
  string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
  set_target_properties(${RESULT_LIB}
    PROPERTIES LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${LIBS_DIR}
    PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${LIBS_DIR}
  )
endforeach()

add_executable(${PROJECT_NAME} ${MODELS_SRC} main.cpp)
target_link_libraries(${PROJECT_NAME}
  # ${RESULT_LIB}
  stdc++fs
  # pugixml site:
  #   http://pugixml.org
  pugixml

  # pqxx site:
  #   http://pqxx.org/development/libpqxx/
  pqxx
  pq
)
