# Тестим всё
message(STATUS "\t\tRun common test")

add_definitions(-DMATH_TEST)

include(${ASP_THERM_CMAKE_ROOT}/models_src.cmake)
include(${ASP_THERM_ROOT}/subprojects/asp_db/db_source.cmake)

add_executable(
  test_common

  ${ASP_THERM_FULLTEST_DIR}/core/common/test_common.cpp
  ${ASP_THERM_FULLTEST_DIR}/core/common/test_math.cpp
  ${ASP_THERM_FULLTEST_DIR}/core/common/test_utils.cpp

  ${MODELS_SRC}
  ${UTILS_SOURCE}
  ${DATABASE_SOURCE}
  ${THERMDB_SOURCE_DIR}/atherm_db_tables.cpp
  ${ASP_THERM_ROOT}/source/utils/FileURL.cpp
)

# link pugixml library
set(PUGIXML_DIR "${MODULES_DIR}/pugixml")
include_directories(${PUGIXML_DIR}/src)
link_directories(${ASP_THERM_ROOT}/build/lib/pugixml)
set(PUGIXML_LIB "pugixml")

#   pqxx
set(PQXX_LIBS pqxx pq)


target_link_libraries(test_common

  ${PUGIXML_LIB}
  ${PQXX_LIBS}
  ${GTEST_LIBRARIES}
  ${FULLTEST_LIBRARIES}
)

add_test(test_common "core/test_common")
