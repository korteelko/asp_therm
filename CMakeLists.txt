cmake_minimum_required(VERSION 3.5)
add_compile_options(-std=c++11 -Wall)

set(PROJECT_NAME asp_therm)
project(${PROJECT_NAME})

# set cmake options:
# debug
add_definitions(-D_DEBUG)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
add_definitions(-D_IDE_VSCODE)
# release
# add_definitions(-DNDEBUG)
# set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall")

if(MSVC)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

set(THERMCORE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/source/core")

set(THERMCORE_INCLUDES
    "${THERMCORE_SOURCE_DIR}/common"
    "${THERMCORE_SOURCE_DIR}/gas_parameters"
    "${THERMCORE_SOURCE_DIR}/models"
    "${THERMCORE_SOURCE_DIR}/phase_diagram"
    "${THERMCORE_SOURCE_DIR}/subroutins"
)

set(OTHER_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(LIBS_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/libs)

INCLUDE_DIRECTORIES(${THERMCORE_INCLUDES})
INCLUDE_DIRECTORIES(${OTHER_INCLUDES})

if(WIN32)
  INCLUDE_DIRECTORIES("${OTHER_INCLUDES}/target_sys/win")
  set(LIBS_DIR
     ${LIBS_DIR}/win)
else()
#if(LINUX)
  INCLUDE_DIRECTORIES("${OTHER_INCLUDES}/target_sys/_nix")
  set(LIBS_DIR
     ${LIBS_DIR}/_nix)
endif()
# else()
#  message(FATAL_ERROR "unknowing system")
# endif()

#find_package(Boost REQUIRED)

link_directories(${LIBS_DIR})

if(CMAKE_BUILD_TYPE=Release)
  link_directories(${LIBS_DIR}/release)
else()
  # add_definitions(-D_DEBUG)
  link_directories(${LIBS_DIR}/debug)
endif()

set(
  MODELS_SRC
# common sources
  ${THERMCORE_SOURCE_DIR}/common/models_errors.cpp
  ${THERMCORE_SOURCE_DIR}/common/models_math.cpp

# gas_parameters sources
  ${THERMCORE_SOURCE_DIR}/gas_parameters/gas_description.cpp
  ${THERMCORE_SOURCE_DIR}/gas_parameters/gas_description_dynamic.cpp
  ${THERMCORE_SOURCE_DIR}/gas_parameters/gas_description_static.cpp
  ${THERMCORE_SOURCE_DIR}/gas_parameters/gas_mix_init.cpp
  ${THERMCORE_SOURCE_DIR}/gas_parameters/gas_ng_gost_defines.cpp

# phase_diagram sources
  ${THERMCORE_SOURCE_DIR}/phase_diagram/phase_diagram.cpp
  ${THERMCORE_SOURCE_DIR}/phase_diagram/phase_diagram_models.cpp

# subroutins sources
  ${THERMCORE_SOURCE_DIR}/subroutins/filereader.cpp
  ${THERMCORE_SOURCE_DIR}/subroutins/xmlreader.cpp

# models sources
  ${THERMCORE_SOURCE_DIR}/models/model_general.cpp
  ${THERMCORE_SOURCE_DIR}/models/model_ideal_gas.cpp
  ${THERMCORE_SOURCE_DIR}/models/model_ng_gost.cpp
  ${THERMCORE_SOURCE_DIR}/models/model_peng_robinson.cpp
  ${THERMCORE_SOURCE_DIR}/models/model_redlich_kwong.cpp
  ${THERMCORE_SOURCE_DIR}/models/models_creator.cpp

# plot sources
# maybe OpenGL libs
)

# output for static libraries
add_library(lmodels STATIC ${MODELS_SRC})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${LIBS_DIR}/debug)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${LIBS_DIR}/release)

set_target_properties(lmodels PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${LIBS_DIR}/debug)

add_executable(${PROJECT_NAME} ${MODELS_SRC} main.cpp)
target_link_libraries(${PROJECT_NAME}
  # lmodels
    # pugixml site:
    #   http://pugixml.org
  pugixml
)
