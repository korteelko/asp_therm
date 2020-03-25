find_package(GTest)
find_package(Threads)
message(STATUS "\tlibs ${GTEST_LIBRARIES}")
message(STATUS "\tincludes ${GTEST_INCLUDE_DIRS}")
if(("${GTEST_LIBRARIES}" STREQUAL "") OR ("${GTEST_INCLUDE_DIRS}" STREQUAL ""))
  set(GTEST_ROOT /opt/gtest/build)
  set(GTEST_INCLUDE_DIR /opt/gtest/googletest/include)
  message(STATUS "gtest libraries wasn't found at standard paths."
    " Try to search in directory: ${GTEST_ROOT}. Specify another path in "
    "${CMAKE_CURRENT_LIST_FILE}")
  find_package(GTest REQUIRED)
else()
  message(STATUS "gtest libraries was found at standard paths")
endif()

if(("${GTEST_LIBRARIES}" STREQUAL "") OR ("${GTEST_INCLUDE_DIRS}" STREQUAL ""))
  message(SEND_ERROR "gtest libraries wasn't found!")
endif()
