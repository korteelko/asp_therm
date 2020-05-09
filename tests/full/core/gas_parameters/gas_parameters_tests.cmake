message(STATUS "\t\tRun gasmix test")

add_definitions(-DGASMIX_TEST)
add_executable(
  test_gasmix

  ${ASP_THERM_FULLTEST_DIR}/core/gas_parameters/test_gasmix.cpp

  ${THERMCORE_SOURCE_DIR}/common/common.cpp
  ${THERMCORE_SOURCE_DIR}/common/merror_codes.cpp
  ${THERMCORE_SOURCE_DIR}/common/models_math.cpp

  ${THERMCORE_SOURCE_DIR}/gas_parameters/gas_description.cpp
  ${THERMCORE_SOURCE_DIR}/gas_parameters/gas_description_dynamic.cpp
  ${THERMCORE_SOURCE_DIR}/gas_parameters/gas_description_static.cpp
  ${THERMCORE_SOURCE_DIR}/gas_parameters/gasmix_init.cpp

  ${THERMCORE_SOURCE_DIR}/subroutins/file_structs.cpp

  ${THERMUTILS_SOURCE_DIR}/ErrorWrap.cpp
  ${THERMUTILS_SOURCE_DIR}/Logging.cpp
)

set(PUGIXML_DIR "${MODULES_DIR}/pugixml")
include_directories(${PUGIXML_DIR}/src)
link_directories(${ASP_THERM_ROOT}/build/lib/pugixml)
set(PUGIXML_LIB "pugixml")
#find_library(${ASP_THERM_ROOT}/build/lib/pugixml ${PUGIXML_LIB})

target_link_libraries(test_gasmix

  ${PUGIXML_LIB}
  ${GTEST_LIBRARIES}
  ${FULLTEST_LIBRARIES}
)

add_test(test_gasmix "core/test_gasmix")
