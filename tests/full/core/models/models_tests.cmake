message(STATUS "\t\tRun models test")

add_executable(test_models
  ${ASP_THERM_FULLTEST_DIR}/core/models/test_models_base.cpp
  ${THERMCORE_SOURCE_DIR}/common/models_math.cpp
  ${THERMCORE_SOURCE_DIR}/gas_parameters/gas_description.cpp
  ${THERMCORE_SOURCE_DIR}/service/calculation_info.cpp)

target_compile_definitions(test_models
  PRIVATE -DBYCMAKE_DEBUG -DTESTING_PROJECT -DMODELS_TEST ${INCLUDE_ERRORCODES})
target_compile_options(test_models PRIVATE -fprofile-arcs -ftest-coverage)
target_include_directories(test_models
  PRIVATE ${TESTS_INCLUDE_DIRS}
  PRIVATE ${MODULES_DIR}/asp_db/source)
target_link_libraries(test_models asp_utils ${FULLTEST_LIBRARIES})

add_test(test_models "core/test_models")
