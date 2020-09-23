message(STATUS "\t\tRun gasmix test")

set(TEST_NAME test_gasparameters)

add_definitions(-DGASMIX_TEST)
add_definitions(-DISO_20765)


add_executable(
  ${TEST_NAME}

  ${ASP_THERM_FULLTEST_DIR}/core/gas_parameters/test_gasmix.cpp
  ${ASP_THERM_FULLTEST_DIR}/core/gas_parameters/test_gas_description.cpp

  ${THERMCORE_SOURCE_DIR}/common/atherm_common.cpp
  ${THERMCORE_SOURCE_DIR}/common/models_math.cpp

  ${THERMCORE_SOURCE_DIR}/gas_parameters/gas_description.cpp
  ${THERMCORE_SOURCE_DIR}/gas_parameters/gas_description_dynamic.cpp
  ${THERMCORE_SOURCE_DIR}/gas_parameters/gas_description_static.cpp
  ${THERMCORE_SOURCE_DIR}/gas_parameters/gasmix_init.cpp

  ${THERMCORE_SOURCE_DIR}/subroutins/file_structs.cpp
)

set(PUGIXML_DIR "${MODULES_DIR}/pugixml")
include_directories(${PUGIXML_DIR}/src)
link_directories(${ASP_THERM_ROOT}/build/lib/pugixml)
set(PUGIXML_LIB "pugixml")
#find_library(${ASP_THERM_ROOT}/build/lib/pugixml ${PUGIXML_LIB})

target_link_libraries(${TEST_NAME}

  asp_utils
  ${PUGIXML_LIB}
  ${FULLTEST_LIBRARIES}
)

add_test(${TEST_NAME} "core/test_gasmix")
