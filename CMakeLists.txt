cmake_minimum_required(VERSION 3.7)
add_compile_options(-std=c++17 -Wall)

set(PROJECT_NAME asp_therm)
project(${PROJECT_NAME})

# set cmake options:
# debug
add_definitions(-D_DEBUG)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
# release
# add_definitions(-DNDEBUG)
# set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall")

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

include_directories(${THERMCORE_INCLUDES})
include_directories(${OTHER_INCLUDES})

# ????????????????????????????????????????
# construction don't work with configuration:
#  if(MSVC)
#    ...
#  elseif(UNIX)
#    ...
#  endif(MSVS)
# ?????????????????????????????????????????
if(UNIX)
  add_definitions(-DOS_NIX)
  include_directories("${OTHER_INCLUDES}/target_sys/_nix")
  set(LIBS_DIR
     ${LIBS_DIR}/_nix)
elseif(MSVC)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS -DOS_WIN)
  include_directories("${OTHER_INCLUDES}/target_sys/win")
  set(LIBS_DIR
     ${LIBS_DIR}/win)
else()
  message(SEND_ERROR "UNDEFINED SYSTEM ${CMAKE_SYSTEM_NAME}")
endif(UNIX)

link_directories(${LIBS_DIR})

if(CMAKE_BUILD_TYPE=Release)
  link_directories(${LIBS_DIR}/release)
else()
  link_directories(${LIBS_DIR}/debug)
endif()

set(
  MODELS_SRC
# common sources
  ${THERMCORE_SOURCE_DIR}/common/models_common.cpp
  ${THERMCORE_SOURCE_DIR}/common/models_errors.cpp
  ${THERMCORE_SOURCE_DIR}/common/models_logging.cpp
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
  ${THERMCORE_SOURCE_DIR}/subroutins/gas_by_file.cpp
  ${THERMCORE_SOURCE_DIR}/subroutins/gasmix_by_file.cpp
  ${THERMCORE_SOURCE_DIR}/subroutins/xml_reader.cpp

# models sources
  ${THERMCORE_SOURCE_DIR}/models/model_general.cpp
  ${THERMCORE_SOURCE_DIR}/models/model_ideal_gas.cpp
  ${THERMCORE_SOURCE_DIR}/models/model_ng_gost.cpp
  ${THERMCORE_SOURCE_DIR}/models/model_peng_robinson.cpp
  ${THERMCORE_SOURCE_DIR}/models/model_redlich_kwong.cpp
  ${THERMCORE_SOURCE_DIR}/models/models_configurations.cpp
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
