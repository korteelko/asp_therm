# Тестим всё
message(STATUS "\t\tRun common test")

add_definitions(-DMATH_TEST)

include(${ASP_THERM_CMAKE_ROOT}/models_src.cmake)

add_executable(
  test_common

  ${ASP_THERM_FULLTEST_DIR}/core/common/test_math.cpp
  ${ASP_THERM_FULLTEST_DIR}/core/common/test_utils.cpp

  ${UTILS_SOURCE}
  ${ASP_THERM_ROOT}/source/core/common/atherm_common.cpp
  ${ASP_THERM_ROOT}/source/core/common/models_math.cpp
  ${ASP_THERM_ROOT}/source/core/subroutins/file_structs.cpp
)

target_link_libraries(test_common

  asp_utils
  ${FULLTEST_LIBRARIES}
)

add_test(test_common "core/test_common")
