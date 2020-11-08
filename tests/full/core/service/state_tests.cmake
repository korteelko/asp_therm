message(STATUS "\t\tRun full test(state)")

include(${ASP_THERM_CMAKE_ROOT}/models_src.cmake)
add_executable(test_state
  ${MODELS_SRC}

  ${ASP_THERM_FULLTEST_DIR}/core/service/test_state.cpp
  ${ASP_THERM_FULLTEST_DIR}/core/service/test_calculation_setup.cpp

  ${THERMDB_SOURCE_DIR}/atherm_db_tables.cpp)

target_compile_definitions(test_state
  PRIVATE -DBYCMAKE_DEBUG -DTESTING_PROJECT -DISO_20765
  ${INCLUDE_ERRORCODES})
target_compile_options(test_state PRIVATE -fprofile-arcs -ftest-coverage)
target_include_directories(test_state
  PRIVATE ${TESTS_INCLUDE_DIRS}
  PRIVATE ${MODULES_DIR}/asp_db/source)
target_link_libraries(test_state
  pugixml
  pqxx pq
  asp_utils
  asp_db
  ${FULLTEST_LIBRARIES})

add_test(test_state "core/test_state")
