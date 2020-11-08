message(STATUS "\t\tRun gasparameters test")

add_executable(test_gasparameters

  ${ASP_THERM_FULLTEST_DIR}/core/gas_parameters/test_gasmix.cpp
  ${ASP_THERM_FULLTEST_DIR}/core/gas_parameters/test_gas_description.cpp
  ${ASP_THERM_FULLTEST_DIR}/core/gas_parameters/test_gost_ng.cpp

  ${THERMCORE_SOURCE_DIR}/common/atherm_common.cpp
  ${THERMCORE_SOURCE_DIR}/common/models_math.cpp

  ${THERMCORE_SOURCE_DIR}/gas_parameters/gas_description.cpp
  ${THERMCORE_SOURCE_DIR}/gas_parameters/gas_description_dynamic.cpp
  ${THERMCORE_SOURCE_DIR}/gas_parameters/gas_description_static.cpp
  ${THERMCORE_SOURCE_DIR}/gas_parameters/gasmix_init.cpp

  ${THERMCORE_SOURCE_DIR}/subroutins/file_structs.cpp)

target_compile_definitions(test_gasparameters
  PRIVATE -DTESTING_PROJECT -DBYCMAKE_DEBUG -DGASMIX_TEST -DISO_20765
  ${INCLUDE_ERRORCODES})
target_compile_options(test_gasparameters
  PRIVATE -fprofile-arcs -ftest-coverage)
target_include_directories(test_gasparameters
  PRIVATE ${TESTS_INCLUDE_DIRS}
  PRIVATE ${MODULES_DIR}/asp_db/source)
target_link_libraries(test_gasparameters
  pugixml
  asp_utils
  ${FULLTEST_LIBRARIES})

add_test(test_gasparameters "core/test_gasparameters")
