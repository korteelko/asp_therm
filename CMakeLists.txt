cmake_minimum_required(VERSION 3.9)
# CXX17 for gcc available for cmake version > 3.8

set(PROJECT_NAME asp_therm)
set(RESULT_LIB atherm)
project(${PROJECT_NAME})

add_compile_options(-Wall)

add_definitions(-D_DEBUG)
set(CMAKE_BUILD_TYPE Debug)

set(ASP_THERM_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

# BUILD OPTIONS
list(APPEND
  BUILD_OPTIONS

  # use pugixml
  #   now pugi is necessarily, not comment it
  WITH_PUGIXML
  # use rapidjson
  WITH_RAPIDJSON
  # build postgresql module and link pqxx library
  WITH_POSTGRESQL
  # enable tests
  TESTS_ENABLED
)

# set compiler requirements, source subdirectories
#   file of error codes
include(${ASP_THERM_ROOT}/atherm_setup.cmake)

# setup utils
# LIBS_DIR - для библиотеки atherm
set(LIBS_DIR ${ASP_THERM_ROOT}/lib/${RESULT_LIB})
# MODULES_DIR - для сторонних библиотек.
set(MODULES_DIR ${ASP_THERM_ROOT}/lib)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(LIBS_DIR "${LIBS_DIR}/debug")
endif()
link_directories(${LIBS_DIR})


# todo: maybe split in separate asp_therm_modules.cmake ???
# todo: rename function to 'setup' 'modules'? something like this
function(iterate_modules_list)
  message(STATUS "\t${PROJECT_NAME} build configuration:")
  foreach(mod ${ARGN})
    message(STATUS "\t\t${mod}")
    # todo: eto ne rabotaet, no vyglyadit interesno
    # set(${mod} TRUE)
    # В общем, чтобы не искать в коде С++ дефайны заданные
    #   в cmake-файле сгруппируем их там и тут)
    add_definitions(-DBYCMAKE_${mod})
  endforeach()
endfunction()


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

set(MODELS_SRC

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
  ${THERMCORE_SOURCE_DIR}/models/model_redlich_kwong_soave.cpp
  ${THERMCORE_SOURCE_DIR}/models/models_configurations.cpp
  ${THERMCORE_SOURCE_DIR}/models/models_creator.cpp


# utils sources
  ${THERMUTILS_SOURCE_DIR}/ErrorWrap.cpp
  ${THERMUTILS_SOURCE_DIR}/Logging.cpp

# database sources
  ${THERMDB_SOURCE_DIR}/db_connection.cpp
  ${THERMDB_SOURCE_DIR}/db_connection_manager.cpp
  ${THERMDB_SOURCE_DIR}/db_defines.cpp
  ${THERMDB_SOURCE_DIR}/db_query.cpp

# plot sources
# maybe OpenGL libs
)
if(INCLUDE_ERRORCODES)
  LIST(APPEND MODELS_SRC
      ${THERMCORE_SOURCE_DIR}/common/merror_codes.cpp)
endif()


# SETUP BUILD_OPTIONS
iterate_modules_list("${BUILD_OPTIONS}")

# modules
#   pugixml
#   http://pugixml.org
if(WITH_PUGIXML IN_LIST BUILD_OPTIONS)
  set(PUGIXML_DIR "${MODULES_DIR}/pugixml")
  add_subdirectory(${PUGIXML_DIR})
  include_directories(${PUGIXML_DIR}/src)
  set(PUGIXML_LIB "pugixml")
endif()

#   rapidjson
#   https://github.com/Tencent/rapidjson
if(WITH_RAPIDJSON IN_LIST BUILD_OPTIONS)
  # rapidjson header-only
  set(RAPIDJSON_DIR "${MODULES_DIR}/rapidjson")
  add_subdirectory(${RAPIDJSON_DIR})
  include_directories(${RAPIDJSON_DIR}/include)
endif()

#   pqxx
#   http://pqxx.org/development/libpqxx/
if(WITH_POSTGRESQL IN_LIST BUILD_OPTIONS)
  set(PQXX_LIBS pqxx pq)
  list(APPEND MODELS_SRC
      ${THERMDB_SOURCE_DIR}/db_connection_postgre.cpp)
endif()


# build result library
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

# build test executable
add_executable(${PROJECT_NAME} ${MODELS_SRC} main.cpp)

target_link_libraries(${PROJECT_NAME}
  # ${RESULT_LIB}

  stdc++fs
  ${PUGIXML_LIB}
  ${PQXX_LIBS}
)

# run tests
if(ENABLE_TESTS IN_LIST BUILD_OPTIONS)
  add_subdirectory(${ASP_THERM_ROOT}/tests/full)
endif()
