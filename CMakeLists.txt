cmake_minimum_required(VERSION 3.9)

set(PROJECT_NAME asp_therm)
project(${PROJECT_NAME})
# name of result lib(for release build)
set(RESULT_LIB atherm)
# executable target for debugging
set(EXECUTABLE ${PROJECT_NAME}_debug)

set(ASP_THERM_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(ASP_THERM_CMAKE_ROOT ${ASP_THERM_ROOT}/cmake)

option(WITH_PUGIXML "Build with pugixml(always on)" ON)
option(ISO_20765 "Include iso 20765 to build" ON)
option(WITH_POSTGRESQL "Build with postres libs: `pq` and `pqxx`" ON)
option(TESTS_ENABLED "Run tests" ON)

# setup cmake directories
set(THERMCORE_SOURCE_DIR "${ASP_THERM_ROOT}/source/core")
set(THERMDB_SOURCE_DIR "${ASP_THERM_ROOT}/source/database")
set(THERMREADER_SOURCE_DIR "${ASP_THERM_ROOT}/source/reader")

# setup file paths and cmake error codes
set(ERRORCODES_FILE "merror_codes.h")
set(ERRORCODES_PATH "${THERMCORE_SOURCE_DIR}/common/${ERRORCODES_FILE}")
if(EXISTS ${ERRORCODES_PATH})
  message(STATUS "Add file with codes of errors: ${ERRORCODES_PATH}")
  set(INCLUDE_ERRORCODES -DINCLUDE_ERRORCODES)
endif()

# TODO: move it to env setup .cmake file
if(MSVC)
  message(STATUS "asp_therm add define: -D_CRT_SECURE_NO_WARNINGS")
  list(APPEND DEFINES_LIST "-D_CRT_SECURE_NO_WARNINGS")
endif()

# setup utils OUT_DIR - для библиотеки atherm
set(OUT_DIR ${ASP_THERM_ROOT}/out/${RESULT_LIB})
# MODULES_DIR - для сторонних библиотек.
set(MODULES_DIR ${ASP_THERM_ROOT}/lib)

if(CMAKE_BUILD_TYPE MATCHES Debug)
  set(OUT_DIR ${OUT_DIR}/debug)
endif()

message(STATUS "asp_therm cmake build info:")
message(STATUS "\t\tSystem info: ${CMAKE_SYSTEM_NAME}")
message(STATUS "\t\tBUILD_TYPE: ${CMAKE_BUILD_TYPE}")
message(STATUS "\t\tOUT_DIR: ${OUT_DIR}")

set(THERMCORE_INCLUDES
    "${THERMCORE_SOURCE_DIR}/common" "${THERMCORE_SOURCE_DIR}/gas_parameters"
    "${THERMCORE_SOURCE_DIR}/models" "${THERMCORE_SOURCE_DIR}/phase_diagram"
    "${THERMCORE_SOURCE_DIR}/service" "${THERMCORE_SOURCE_DIR}/subroutins")

set(INCLUDE_DIRS ${THERMCORE_INCLUDES} ${THERMDB_SOURCE_DIR}
                 ${THERMREADER_SOURCE_DIR})

# add pugi xml
add_definitions(-DBYCMAKE_WITH_PUGIXML)
message(STATUS "Add library pugixml."
               "See http://pugixml.org for more information")
add_subdirectory(${MODULES_DIR}/pugixml)
list(APPEND INCLUDE_DIRS ${MODULES_DIR}/pugixml/src)

# add pq, pqxx
if(WITH_POSTGRESQL)
  # set additional information in `asp_db` module
  list(APPEND DEFINES_LIST "-DBYCMAKE_WITH_POSTGRESQL")
endif()
if(ISO_20765)
  list(APPEND DEFINES_LIST "-DBYCMAKE_ISO_20765")
endif()

add_subdirectory(${MODULES_DIR}/asp_db)
find_package(Threads REQUIRED)
set(LIBRARIES Threads::Threads pugixml asp_db)

include(${ASP_THERM_CMAKE_ROOT}/models_src.cmake)
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

if(${CMAKE_BUILD_TYPE} MATCHES Debug)
  # build test executable
  add_executable(${EXECUTABLE} ${MODELS_SRC} main.cpp)

  target_compile_options(${EXECUTABLE} PRIVATE -Wall -fno-stack-protector)
  target_compile_definitions(
    ${EXECUTABLE} PRIVATE -DBYCMAKE_DEBUG ${INCLUDE_ERRORCODES} ${DEFINES_LIST})
  target_include_directories(${EXECUTABLE} PRIVATE ${INCLUDE_DIRS})
  target_link_libraries(${EXECUTABLE} ${LIBRARIES})
endif()

# Генерировать compile_commands.json файл компилируемых объектов по которым ccls
# будет правильно инклуды/дефайны расставлять
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# Скопировать compile_commands в корень проекта
add_custom_target(
  copy-compile-commands ALL
  ${CMAKE_COMMAND} -E copy_if_different
  ${CMAKE_BINARY_DIR}/compile_commands.json ${CMAKE_CURRENT_SOURCE_DIR})

# run tests
if(TESTS_ENABLED)
  add_subdirectory(${ASP_THERM_ROOT}/tests/full)
endif()
