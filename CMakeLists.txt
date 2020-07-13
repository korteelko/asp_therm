cmake_minimum_required(VERSION 3.9)

set(PROJECT_NAME asp_therm)
set(RESULT_LIB atherm)
project(${PROJECT_NAME})

add_compile_options(-Wall)

# todo: both options also can be placed in 'BUILD_OPTION' list
add_definitions(-DBYCMAKE_DEBUG)
add_definitions(-DBYCMAKE_CXX17)
set(CMAKE_BUILD_TYPE Debug)

set(ASP_THERM_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(ASP_THERM_CMAKE_ROOT ${ASP_THERM_ROOT}/cmake)
set(ASP_DB_ROOT ${ASP_THERM_ROOT}/subprojects/asp_db)

# todo: maybe split in separate asp_therm_modules.cmake ???
# todo: add full clear step(ninja cannot clear
#   rapidjson 'doc/html' dir by yourself)
#======================================================
# SET BUILD OPTIONS
#======================================================
function(set_build_options)
  message(STATUS "\t${PROJECT_NAME} build configuration:")
  foreach(mod ${ARGN})
    message(STATUS "\t\t${mod}")
    # прокинуть дефайн вида 'BYCMAKE_${OPTION}' компилятору.
    #   Префикс BYCMAKE для удобства поиска по проекту
    add_definitions(-DBYCMAKE_${mod})
    set(${mod} TRUE PARENT_SCOPE)
  endforeach()
endfunction()


list(APPEND
  BUILD_OPTIONS

  # use pugixml
  #   now pugi is necessarily, not comment it
  WITH_PUGIXML
  # build postgresql module and link pqxx library
  WITH_POSTGRESQL
  # enable tests
  TESTS_ENABLED
)

# SETUP BUILD_OPTIONS
set_build_options("${BUILD_OPTIONS}")


#======================================================
# PLATFORM
#======================================================
# set compiler requirements, source subdirectories
#   file of error codes
include(${ASP_THERM_CMAKE_ROOT}/atherm_setup.cmake)

# setup utils
# LIBS_DIR - для библиотеки atherm
set(LIBS_DIR ${ASP_THERM_ROOT}/lib/${RESULT_LIB})
# MODULES_DIR - для сторонних библиотек.
set(MODULES_DIR ${ASP_THERM_ROOT}/lib)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(LIBS_DIR "${LIBS_DIR}/debug")
endif()
link_directories(${LIBS_DIR})

message(STATUS "asp_therm cmake build info:")
message(STATUS "\t\tSystem info: ${CMAKE_SYSTEM_NAME}")
message(STATUS "\t\tBUILD_TYPE: ${CMAKE_BUILD_TYPE}")
message(STATUS "\t\tLIBS_DIR: ${LIBS_DIR}")

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


#======================================================
# SOURCE
#======================================================
# add general sources
# set variable 'MODELS_SRC' - general source files of project
include(${ASP_THERM_CMAKE_ROOT}/models_src.cmake)
# add database sources
include(${ASP_DB_ROOT}/db_source.cmake)
#list(TRANSFORM DATABASE_SOURCE PREPEND " ${ASP_DB_ROOT}/source/")
list(APPEND MODELS_SRC "${DATABASE_SOURCE}")
# add utils sources
include(${ASP_DB_ROOT}/utils_source.cmake)
#list(TRANSFORM UTILS_SOURCE PREPEND ${ASP_DB_ROOT})
list(APPEND MODELS_SRC "${UTILS_SOURCE}")

# utils
list(APPEND MODELS_SRC "${ASP_THERM_ROOT}/source/utils/FileURL.cpp")


#======================================================
# LIBRARIES AND MODULES
#======================================================
# modules
#   pugixml
#   http://pugixml.org
if(WITH_PUGIXML)
  message(STATUS "Add library pugixml")
  set(PUGIXML_DIR "${MODULES_DIR}/pugixml")
  add_subdirectory(${PUGIXML_DIR})
  include_directories(${PUGIXML_DIR}/src)
  set(PUGIXML_LIB "pugixml")
endif()

#   pqxx
#   http://pqxx.org/development/libpqxx/
if(WITH_POSTGRESQL)
  set(PQXX_LIBS pqxx pq)
else()
  message(WARNING "Exclude all .cpp files matched with regex: '*postgre.cpp'")
  list(FILTER MODEL_SRC EXCLUDE REGEX "postgre.cpp")
endif()

#   thread
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)


#======================================================
# OUTPUT SETUP
#======================================================
# build result library
if (${CMAKE_BUILD_TYPE} STREQUAL "Release")
  add_library(${RESULT_LIB} STATIC
    ${MODELS_SRC}
    ${DATABASE_SOURCE}
    ${UTILS_SOURCE}
  )
  set_target_properties(${RESULT_LIB} PROPERTIES
      LIBRARY_OUTPUT_DIRECTORY ${LIBS_DIR}
      ARCHIVE_OUTPUT_DIRECTORY ${LIBS_DIR}
  )
endif()

foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
  string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
  set_target_properties(${RESULT_LIB}
    PROPERTIES LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${LIBS_DIR}
    PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${LIBS_DIR}
  )
endforeach()

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
  # build test executable
  add_executable(${PROJECT_NAME}
    ${MODELS_SRC}
    ${DATABASE_SOURCE}
    ${UTILS_SOURCE}
    main.cpp
  )
endif()

target_link_libraries(${PROJECT_NAME}
  # ${RESULT_LIB}

  stdc++fs
  ${PUGIXML_LIB}
  ${PQXX_LIBS}
  Threads::Threads
)

# run tests
if(TESTS_ENABLED)
  add_subdirectory(${ASP_THERM_ROOT}/tests/full)
endif()
