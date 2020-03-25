message(STATUS "\t\tRun math test")

add_definitions(-DMATH_TEST)
add_executable(
  test_math

  ${ASP_THERM_FULLTEST_DIR}/core/common/test_math.cpp

  ${THERMCORE_SOURCE_DIR}/common/models_math.cpp
)

target_link_libraries(test_math ${GTEST_LIBRARIES} Threads::Threads)

add_test(test_math "core/test_math")
