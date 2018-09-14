cmake_minimum_required(VERSION 3.5)
add_compile_options(-std=c++11 -Wall)

set(PROJECT_NAME asp_therm)
project(${PROJECT_NAME})

# set cmake options:
# debug
# set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall")
# release
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall")

if(MSVC)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()


set (THERMCORE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/source/core")

set (THERMCORE_INCLUDES
    "${THERMCORE_SOURCE_DIR}/common"
    "${THERMCORE_SOURCE_DIR}/gas_parameters"
    "${THERMCORE_SOURCE_DIR}/models"
    "${THERMCORE_SOURCE_DIR}/phase_diagram"
)

set(OTHER_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(LIBS_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/libs)

INCLUDE_DIRECTORIES(${THERMCORE_INCLUDES})
INCLUDE_DIRECTORIES(${OTHER_INCLUDES})
if (WIN32)
  INCLUDE_DIRECTORIES("${OTHER_INCLUDES}/target_sys/win")
#endif(WIN32)
else ()
#if(LINUX)
  INCLUDE_DIRECTORIES("${OTHER_INCLUDES}/target_sys/_nix")
endif()
# else()
#  message(FATAL_ERROR "unknowing system")
# endif()

#find_package(Boost REQUIRED)

link_directories(${LIBS_DIR})

if (CMAKE_BUILD_TYPE=Release)
  link_directories(${LIBS_DIR}/release)
else()
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
  ${THERMCORE_SOURCE_DIR}/gas_parameters/init_by_file.cpp

# phase_diagram sources
  ${THERMCORE_SOURCE_DIR}/phase_diagram/phase_diagram.cpp
  ${THERMCORE_SOURCE_DIR}/phase_diagram/phase_diagram_models.cpp

# subroutins sources
  ${THERMCORE_SOURCE_DIR}/subroutins/filereader.cpp
  ${THERMCORE_SOURCE_DIR}/subroutins/xmlreader.cpp

# models sources
  ${THERMCORE_SOURCE_DIR}/models/model_general.cpp
  ${THERMCORE_SOURCE_DIR}/models/model_ideal_gas.cpp
  ${THERMCORE_SOURCE_DIR}/models/model_peng_robinson.cpp
  ${THERMCORE_SOURCE_DIR}/models/model_redlich_kwong.cpp
  ${THERMCORE_SOURCE_DIR}/models/models_creator.cpp
)

# output for static libraries
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${LIBS_DIR}/debug)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${LIBS_DIR}/release)

add_library(lmodels STATIC ${MODELS_SRC})
add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME}
  lmodels
    # pugixml site:
    #   http://pugixml.org
  pugixml
)
