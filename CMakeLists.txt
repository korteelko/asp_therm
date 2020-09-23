cmake_minimum_required(VERSION 3.9)

set(PROJECT_NAME asp_therm)
set(RESULT_LIB atherm)
set(EXECUTABLE ${PROJECT_NAME}_debug)
project(${PROJECT_NAME})

add_compile_options(-Wall)

# todo: both options also can be placed in 'BUILD_OPTION' list
set(CMAKE_BUILD_TYPE Debug)
add_definitions(-DBYCMAKE_DEBUG)
add_definitions(-DBYCMAKE_CXX17)

set(ASP_THERM_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(ASP_THERM_CMAKE_ROOT ${ASP_THERM_ROOT}/cmake)

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
add_definitions(-DISO_20765)


#======================================================
# PLATFORM
#======================================================
# set compiler requirements, source subdirectories
#   file of error codes
include(${ASP_THERM_CMAKE_ROOT}/atherm_setup.cmake)

# setup utils
# OUT_DIR - для библиотеки atherm
set(OUT_DIR ${ASP_THERM_ROOT}/out/${RESULT_LIB})
# MODULES_DIR - для сторонних библиотек.
set(MODULES_DIR ${ASP_THERM_ROOT}/lib)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(OUT_DIR "${OUT_DIR}/debug")
endif()

message(STATUS "asp_therm cmake build info:")
message(STATUS "\t\tSystem info: ${CMAKE_SYSTEM_NAME}")
message(STATUS "\t\tBUILD_TYPE: ${CMAKE_BUILD_TYPE}")
message(STATUS "\t\tOUT_DIR: ${OUT_DIR}")

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
  ${THERMREADER_SOURCE_DIR}
)


#======================================================
# SOURCE
#======================================================
# add general sources
# set variable 'MODELS_SRC' - general source files of project
include(${ASP_THERM_CMAKE_ROOT}/models_src.cmake)


#======================================================
# LIBRARIES AND MODULES
#======================================================
add_subdirectory(${MODULES_DIR}/asp_db)
include_directories(${MODULES_DIR}/asp_db/source)
# todo: переделать, не очень красиво
include_directories(${MODULES_DIR}/asp_db/lib/asp_utils/source)
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
endif()

#   thread
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)


#======================================================
# OUTPUT SETUP
#======================================================
foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
  string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
  set_target_properties(${RESULT_LIB}
    PROPERTIES LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${OUT_DIR}
    PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${OUT_DIR}
  )
endforeach()
set(LIBRARIES

  asp_db
  ${PUGIXML_LIB}
  ${PQXX_LIBS}
  Threads::Threads
)
if (${CMAKE_BUILD_TYPE} STREQUAL "Release")
  # build result library
  # todo: check result
  add_library(${RESULT_LIB} STATIC ${MODELS_SRC})
  set_target_properties(${RESULT_LIB} PROPERTIES
      LIBRARY_OUTPUT_DIRECTORY ${OUT_DIR}
      ARCHIVE_OUTPUT_DIRECTORY ${OUT_DIR}
  )
  target_link_libraries(${RESULT_LIB} ${LIBRARIES})
endif()

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
  # build test executable
  add_executable(${EXECUTABLE}

    ${MODELS_SRC}
    main.cpp
  )
  target_link_libraries(${EXECUTABLE} ${LIBRARIES})
endif()


# run tests
if(TESTS_ENABLED)
  add_subdirectory(${ASP_THERM_ROOT}/tests/full)
endif()
