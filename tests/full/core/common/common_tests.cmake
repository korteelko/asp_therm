# Тестим всё
message(STATUS "\t\tRun common test")

add_definitions(-DMATH_TEST)

include(${ASP_THERM_CMAKE_ROOT}/models_src.cmake)

add_executable(
  test_common

  ${ASP_THERM_FULLTEST_DIR}/core/common/test_common.cpp
  ${ASP_THERM_FULLTEST_DIR}/core/common/test_math.cpp
  ${ASP_THERM_FULLTEST_DIR}/core/common/test_utils.cpp

  ${MODELS_SRC}
  ${THERMCORE_SOURCE_DIR}/common/merror_codes.cpp
  ${THERMDB_SOURCE_DIR}/atherm_db_tables.cpp
  ${THERMDB_SOURCE_DIR}/db_connection_postgre.cpp
)

# link pugixml library
set(PUGIXML_DIR "${MODULES_DIR}/pugixml")
include_directories(${PUGIXML_DIR}/src)
link_directories(${ASP_THERM_ROOT}/build/lib/pugixml)
set(PUGIXML_LIB "pugixml")

#   pqxx
set(PQXX_LIBS pqxx pq)
list(APPEND MODELS_SRC
    ${THERMDB_SOURCE_DIR}/db_connection_postgre.cpp)


target_link_libraries(test_common

  ${PUGIXML_LIB}
  ${PQXX_LIBS}
  ${GTEST_LIBRARIES}
  ${FULLTEST_LIBRARIES}
)

add_test(test_common "core/test_common")
