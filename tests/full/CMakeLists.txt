cmake_minimum_required(VERSION 3.7)

project(asp_therm-fulltests)

# so intresting and it's all about cmake and gtest!
#   https://cmake.org/cmake/help/latest/module/GoogleTest.html

# setup gtest tests
#   maybe
# todo: replace with more common method
set(GTEST_ROOT /opt/gtest/build)
set(GTEST_INCLUDE_DIR /opt/gtest/googletest/include)
find_package(GTest REQUIRED)
find_package(Threads REQUIRED)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(ASP_THERM_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../)
set(THERMCORE_SOURCE_DIR "${ASP_THERM_ROOT}/source/core")
# set(THERMDB_SOURCE_DIR "${ASP_THERM_ROOT}/source/database")
set(THERMUTILS_SOURCE_DIR "${ASP_THERM_ROOT}/source/utils")

set(AST_THERM_FULLTEST_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# setup file paths and cmake
#   tests
set(THERMTEST_CMAKEFILE "${ASP_THERM_ROOT}/tests_run.cmake")
#   error codes
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

# includes
include_directories(
  ${THERMCORE_SOURCE_DIR}/common
  ${THERMUTILS_SOURCE_DIR}
  ${GTEST_INCLUDE_DIRS}
)

# core testes
add_executable(
  test_math
  ${AST_THERM_FULLTEST_DIR}/core/test_math.cpp
  ${THERMCORE_SOURCE_DIR}/common/models_math.cpp
)

target_link_libraries(test_math ${GTEST_LIBRARIES} Threads::Threads)

enable_testing()
add_test(test_math "core/test_math")
