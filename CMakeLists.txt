cmake_minimum_required(VERSION 3.9)

set(PROJECT_NAME asp_therm)
project(${PROJECT_NAME})
# name of result lib(for release build)
set(RESULT_LIB atherm)
# executable target for debugging
set(EXECUTABLE ${PROJECT_NAME}_debug)

set(ASP_THERM_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(ASP_THERM_CMAKE_ROOT ${ASP_THERM_ROOT}/cmake)

include(${ASP_THERM_CMAKE_ROOT}/env_setup.cmake)
include(${ASP_THERM_CMAKE_ROOT}/models_src.cmake)

option(ISO_20765 "Include iso 20765 to build" ON)
option(WITH_POSTGRESQL "Build with postres libs: `pq` and `pqxx`" ON)
option(WITH_FIREBIRD "Build with firebird lib: `fbclient`" OFF)
option(BUILD_TESTS "Run tests" ON)
option(BUILD_EXECUTABLE "Run tests" ON)

# setup cmake directories
set(THERMCORE_SOURCE_DIR "${ASP_THERM_ROOT}/source/core")
set(THERMDB_SOURCE_DIR "${ASP_THERM_ROOT}/source/database")

# setup file paths and cmake error codes
set(ERRORCODES_FILE "merror_codes.h")
set(ERRORCODES_PATH "${THERMCORE_SOURCE_DIR}/common/${ERRORCODES_FILE}")
if(EXISTS ${ERRORCODES_PATH})
  message(STATUS "Add file with codes of errors: ${ERRORCODES_PATH}")
  set(INCLUDE_ERRORCODES -DINCLUDE_ERRORCODES)
endif()

# setup utils OUT_DIR - для библиотеки atherm
set(OUT_DIR ${ASP_THERM_ROOT}/out/${RESULT_LIB})
# MODULES_DIR - для сторонних библиотек.
set(MODULES_DIR ${ASP_THERM_ROOT}/lib)

if(CMAKE_BUILD_TYPE MATCHES Debug)
  set(OUT_DIR ${OUT_DIR}/debug)
  set(CMAKE_MESSAGE_LOG_LEVEL TRACE)
endif()

message(STATUS "asp_therm cmake build info:")
message(STATUS "\t\tSystem info: ${CMAKE_SYSTEM_NAME}")
message(STATUS "\t\tBUILD_TYPE: ${CMAKE_BUILD_TYPE}")
message(STATUS "\t\tOUT_DIR: ${OUT_DIR}")

set_include_dirs(INCLUDE_DIRS)
append_list_defines(DEFINES_LIST)
set_models_src(MODELS_SRC)

add_subdirectory(${MODULES_DIR}/asp_db)
find_package(Threads REQUIRED)
set(LIBRARIES Threads::Threads asp_db)

if(${CMAKE_BUILD_TYPE} MATCHES Release)
  # build result library todo: check result
  add_library(${RESULT_LIB} STATIC ${MODELS_SRC})
  set_target_properties(
    ${RESULT_LIB} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${OUT_DIR}
                             ARCHIVE_OUTPUT_DIRECTORY ${OUT_DIR})
  target_compile_definitions(${RESULT_LIB} PRIVATE ${INCLUDE_ERRORCODES}
                                                   ${DEFINES_LIST})
  target_include_directories(${RESULT_LIB} PRIVATE ${INCLUDE_DIRS})
  target_link_libraries(${RESULT_LIB} ${LIBRARIES})
endif()

if(BUILD_EXECUTABLE)
  # build test executable
  add_executable(${EXECUTABLE} ${MODELS_SRC} examples/default/main.cpp)

  target_compile_options(${EXECUTABLE} PRIVATE -Wall)
  target_compile_definitions(
    ${EXECUTABLE} PRIVATE -DBYCMAKE_DEBUG ${INCLUDE_ERRORCODES} ${DEFINES_LIST})
  target_include_directories(${EXECUTABLE} PRIVATE ${INCLUDE_DIRS})
  target_link_libraries(${EXECUTABLE} ${LIBRARIES})
endif()

post_build()

# run tests
if(TESTS_ENABLED)
  add_subdirectory(${ASP_THERM_ROOT}/tests/full)
endif()
