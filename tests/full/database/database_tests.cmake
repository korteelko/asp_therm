message(STATUS "\t\tRun Database tests")

add_definitions(-DDATABASE_TEST)
set (COMMON_SRC
  ${THERMCORE_SOURCE_DIR}/common/common.cpp
  ${THERMCORE_SOURCE_DIR}/common/merror_codes.cpp
  ${THERMCORE_SOURCE_DIR}/common/models_math.cpp

  ${THERMCORE_SOURCE_DIR}/models/calculation_info.cpp

  ${THERMCORE_SOURCE_DIR}/subroutins/file_structs.cpp

  ${THERMDB_SOURCE_DIR}/db_connection.cpp
  ${THERMDB_SOURCE_DIR}/db_connection_manager.cpp
  ${THERMDB_SOURCE_DIR}/db_connection_postgre.cpp
  ${THERMDB_SOURCE_DIR}/db_defines.cpp
  ${THERMDB_SOURCE_DIR}/db_queries_setup.cpp
  ${THERMDB_SOURCE_DIR}/db_query.cpp

  ${THERMUTILS_SOURCE_DIR}/ErrorWrap.cpp
  ${THERMUTILS_SOURCE_DIR}/FileURL.cpp
  ${THERMUTILS_SOURCE_DIR}/Logging.cpp
)
# tests database
add_executable(
  test_database

  ${ASP_THERM_FULLTEST_DIR}/database/test_tables.cpp
  ${COMMON_SRC}
)

target_link_libraries(test_database

  ${GTEST_LIBRARIES}
  Threads::Threads
  pqxx
  pq
)

add_test(test_database "database")
