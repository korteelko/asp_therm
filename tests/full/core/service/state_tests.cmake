# Тестим всё
set(PROJECT_NAME test_state)

message(STATUS "\t\tRun full test")

include(${ASP_THERM_CMAKE_ROOT}/models_src.cmake)

add_executable(
  ${PROJECT_NAME}

  ${ASP_THERM_FULLTEST_DIR}/core/service/test_state.cpp
  ${ASP_THERM_FULLTEST_DIR}/core/service/test_calculation_setup.cpp

  ${MODELS_SRC}
  ${THERMDB_SOURCE_DIR}/atherm_db_tables.cpp
)

# link pugixml library
set(PUGIXML_DIR "${MODULES_DIR}/pugixml")
include_directories(${PUGIXML_DIR}/src)
link_directories(${ASP_THERM_ROOT}/build/lib/pugixml)
set(PUGIXML_LIB "pugixml")

#   pqxx
set(PQXX_LIBS pqxx pq)


target_link_libraries(${PROJECT_NAME}

  asp_utils
  asp_db
  ${PUGIXML_LIB}
  ${PQXX_LIBS}
  ${FULLTEST_LIBRARIES}
)

add_test(${PROJECT_NAME} "core/${PROJECT_NAME}")

