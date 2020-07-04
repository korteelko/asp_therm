message(STATUS "\t\tRun Database tests")

add_definitions(-DDATABASE_TEST)
set (COMMON_SRC
  ${THERMCORE_SOURCE_DIR}/common/atherm_common.cpp
  ${THERMCORE_SOURCE_DIR}/common/merror_codes.cpp
  ${THERMCORE_SOURCE_DIR}/common/models_math.cpp

  ${THERMCORE_SOURCE_DIR}/gas_parameters/gas_description.cpp

  ${THERMCORE_SOURCE_DIR}/models/models_configurations.cpp

  ${THERMCORE_SOURCE_DIR}/service/calculation_info.cpp

  ${THERMCORE_SOURCE_DIR}/subroutins/file_structs.cpp

  ${THERMDB_SOURCE_DIR}/atherm_db_tables.cpp
  ${THERMDB_SOURCE_DIR}/db_connection.cpp
  ${THERMDB_SOURCE_DIR}/db_connection_manager.cpp
  ${THERMDB_SOURCE_DIR}/db_connection_postgre.cpp
  ${THERMDB_SOURCE_DIR}/db_defines.cpp
  ${THERMDB_SOURCE_DIR}/db_queries_setup.cpp
  ${THERMDB_SOURCE_DIR}/db_query.cpp

  ${THERMUTILS_SOURCE_DIR}/Common.cpp
  ${THERMUTILS_SOURCE_DIR}/ErrorWrap.cpp
  ${THERMUTILS_SOURCE_DIR}/FileURL.cpp
  ${THERMUTILS_SOURCE_DIR}/Logging.cpp
)
# tests database
add_executable(
  test_database

  ${ASP_THERM_FULLTEST_DIR}/database/test_tables.cpp
  ${ASP_THERM_FULLTEST_DIR}/database/test_queries.cpp
  ${COMMON_SRC}
)

target_link_libraries(test_database

  ${GTEST_LIBRARIES}
  ${FULLTEST_LIBRARIES}
  pqxx
  pq
)

add_test(test_database "database")
