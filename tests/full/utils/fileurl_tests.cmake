message(STATUS "\t\tRun FileUrls tests")

set(COMMON_SRC
  ${THERMCORE_SOURCE_DIR}/common/atherm_common.cpp
  ${THERMCORE_SOURCE_DIR}/common/models_math.cpp

  ${THERMCORE_SOURCE_DIR}/subroutins/file_structs.cpp)

add_executable(test_fileurl
  ${ASP_THERM_FULLTEST_DIR}/utils/test_fileurl.cpp
  ${COMMON_SRC})

target_compile_definitions(test_fileurl
  PRIVATE -DBYCMAKE_DEBUG -DTESTING_PROJECT -DFILEURLS_TEST ${INCLUDE_ERRORCODES})
target_compile_options(test_fileurl PRIVATE -fprofile-arcs -ftest-coverage)
target_include_directories(test_fileurl
  PRIVATE ${TESTS_INCLUDE_DIRS}
  PRIVATE ${MODULES_DIR}/asp_db/source)
target_link_libraries(test_fileurl asp_utils ${FULLTEST_LIBRARIES})

add_test(test_fileurl "utils/fileurl")
