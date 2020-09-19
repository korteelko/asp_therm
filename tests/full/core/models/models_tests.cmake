message(STATUS "\t\tRun models test")

add_definitions(-DMODELS_TEST)
add_executable(
  test_models

  ${ASP_THERM_FULLTEST_DIR}/core/models/test_models_base.cpp

  ${THERMCORE_SOURCE_DIR}/common/models_math.cpp

  ${THERMCORE_SOURCE_DIR}/gas_parameters/gas_description.cpp

  ${THERMCORE_SOURCE_DIR}/service/calculation_info.cpp

  ${UTILS_SOURCE}
)
target_link_libraries(test_models asp_utils ${FULLTEST_LIBRARIES})

add_test(test_models "core/test_models")
