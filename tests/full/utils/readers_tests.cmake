message(STATUS "\t\tRun Readers test")

# add_definitions(-DREADERS_TEST)
set (COMMON_SRC
  ${THERMCORE_SOURCE_DIR}/common/atherm_common.cpp
  ${THERMCORE_SOURCE_DIR}/common/models_math.cpp

  ${THERMCORE_SOURCE_DIR}/subroutins/file_structs.cpp

  ${THERMUTILS_SOURCE_DIR}/FileURL.cpp
)
list(APPEND COMMON_SRC ${UTILS_SOURCE})

# test xml parser
add_executable(
  test_xml

  ${ASP_THERM_FULLTEST_DIR}/utils/test_xml.cpp
  ${COMMON_SRC}
)

# link pugixml library
set(PUGIXML_DIR "${MODULES_DIR}/pugixml")
include_directories(${PUGIXML_DIR}/src)
link_directories(${ASP_THERM_ROOT}/build/lib/pugixml)
set(PUGIXML_LIB "pugixml")


target_link_libraries(test_xml

  ${PUGIXML_LIB}
  ${FULLTEST_LIBRARIES}
)

add_test(test_xml "utils xml")
