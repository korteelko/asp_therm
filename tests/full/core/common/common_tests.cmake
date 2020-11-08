message(STATUS "\t\tRun common test")

add_executable(test_common

  ${ASP_THERM_FULLTEST_DIR}/core/common/test_math.cpp
  ${ASP_THERM_FULLTEST_DIR}/core/common/test_utils.cpp

  ${ASP_THERM_ROOT}/source/core/common/atherm_common.cpp
  ${ASP_THERM_ROOT}/source/core/common/models_math.cpp
  ${ASP_THERM_ROOT}/source/core/subroutins/file_structs.cpp)

target_compile_definitions(test_common
  PRIVATE -DMATH_TEST -DBYCMAKE_DEBUG -DTESTING_PROJECT ${INCLUDE_ERRORCODES})
target_compile_options(test_common PRIVATE -fprofile-arcs -ftest-coverage)
target_include_directories(test_common
  PRIVATE ${TESTS_INCLUDE_DIRS}
  PRIVATE ${MODULES_DIR}/asp_db/source)
target_link_libraries(test_common asp_utils ${FULLTEST_LIBRARIES})

add_test(test_common "core/test_common")
