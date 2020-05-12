message(STATUS "\t\tRun common test")

add_definitions(-DMATH_TEST)
add_executable(
  test_common

  ${ASP_THERM_FULLTEST_DIR}/core/common/test_math.cpp
  ${ASP_THERM_FULLTEST_DIR}/core/common/test_utils.cpp

  ${THERMCORE_SOURCE_DIR}/common/merror_codes.cpp
  ${THERMCORE_SOURCE_DIR}/common/models_math.cpp
  ${THERMUTILS_SOURCE_DIR}/ErrorWrap.cpp
  ${THERMUTILS_SOURCE_DIR}/Logging.cpp
)

target_link_libraries(test_common ${GTEST_LIBRARIES} ${FULLTEST_LIBRARIES})

add_test(test_common "core/test_common")
