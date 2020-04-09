message(STATUS "\t\tRun Readers test")

add_definitions(-DREADERS_TEST)
add_executable(
  test_readers

  # тестинг json парсера
  #${ASP_THERM_FULLTEST_DIR}/utils/test_json.cpp
  # тестинг xml парсера
  ${ASP_THERM_FULLTEST_DIR}/utils/test_xml.cpp
  # сравниваем результаты парсинга
  #${ASP_THERM_FULLTEST_DIR}/utils/test_readers.cpp

  ${THERMCORE_SOURCE_DIR}/common/common.cpp
  ${THERMCORE_SOURCE_DIR}/common/merror_codes.cpp
  ${THERMCORE_SOURCE_DIR}/common/models_math.cpp

  ${THERMCORE_SOURCE_DIR}/subroutins/file_structs.cpp

  ${THERMUTILS_SOURCE_DIR}/ErrorWrap.cpp
  ${THERMUTILS_SOURCE_DIR}/Logging.cpp
)

set(PUGIXML_DIR "${MODULES_DIR}/pugixml")
include_directories(${PUGIXML_DIR}/src)
link_directories(${ASP_THERM_ROOT}/build/lib/pugixml)
set(PUGIXML_LIB "pugixml")

target_link_libraries(test_readers

  ${GTEST_LIBRARIES}
  ${PUGIXML_LIB}
  Threads::Threads
)

add_test(test_readers "utils")
