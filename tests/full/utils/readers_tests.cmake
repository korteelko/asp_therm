message(STATUS "\t\tRun Readers test")

add_definitions(-DREADERS_TEST)
set (COMMON_SRC
  ${THERMCORE_SOURCE_DIR}/common/common.cpp
  ${THERMCORE_SOURCE_DIR}/common/merror_codes.cpp
  ${THERMCORE_SOURCE_DIR}/common/models_math.cpp

  ${THERMCORE_SOURCE_DIR}/subroutins/file_structs.cpp

  ${THERMUTILS_SOURCE_DIR}/ErrorWrap.cpp
  ${THERMUTILS_SOURCE_DIR}/FileURL.cpp
  ${THERMUTILS_SOURCE_DIR}/Logging.cpp
)
# test xml parser
add_executable(
  test_xml

  ${ASP_THERM_FULLTEST_DIR}/utils/test_xml.cpp
  ${COMMON_SRC}
)
# test json parser
add_executable(
  test_json

  ${ASP_THERM_FULLTEST_DIR}/utils/inode_imp.cpp
  ${ASP_THERM_FULLTEST_DIR}/utils/test_json.cpp
  ${COMMON_SRC}
)

#add_executable(
#  test_readers

  # сравниваем результаты парсинга
#  ${ASP_THERM_FULLTEST_DIR}/utils/test_readers.cpp
#  ${COMMON_SRC}
#)

# link pugixml library
set(PUGIXML_DIR "${MODULES_DIR}/pugixml")
include_directories(${PUGIXML_DIR}/src)
link_directories(${ASP_THERM_ROOT}/build/lib/pugixml)
set(PUGIXML_LIB "pugixml")

# include rapidjson directory
set(RAPIDJSON_DIR "${MODULES_DIR}/rapidjson")
include_directories(${RAPIDJSON_DIR}/include)

target_link_libraries(test_xml

  ${PUGIXML_LIB}
  ${GTEST_LIBRARIES}
  ${FULLTEST_LIBRARIES}
)
target_link_libraries(test_json ${GTEST_LIBRARIES} ${FULLTEST_LIBRARIES})

add_test(test_xml "utils xml")
add_test(test_json "utils json")
# add_test(test_readers "utils readers")
