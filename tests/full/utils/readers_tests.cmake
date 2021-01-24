message(STATUS "\t\tRun Readers tests")

set(COMMON_SRC
  ${THERMCORE_SOURCE_DIR}/common/atherm_common.cpp

  ${THERMCORE_SOURCE_DIR}/subroutins/file_structs.cpp)

add_executable(test_xml
  ${ASP_THERM_FULLTEST_DIR}/utils/test_xml.cpp
  ${COMMON_SRC})

target_compile_definitions(test_xml
  PRIVATE -DBYCMAKE_DEBUG -DTESTING_PROJECT ${INCLUDE_ERRORCODES})
target_compile_options(test_xml PRIVATE -fprofile-arcs -ftest-coverage)
target_include_directories(test_xml
  PRIVATE ${TESTS_INCLUDE_DIRS}
  PRIVATE ${MODULES_DIR}/asp_db/source)
target_link_libraries(test_xml pugixml asp_utils ${FULLTEST_LIBRARIES})

add_test(test_xml "utils/xml")
