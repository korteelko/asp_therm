if(NOT EXISTS ${ASP_THERM_ROOT})
  message(FATAL_ERROR "ошибка конфигурации cmake")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# setup cmake directories
set(THERMCORE_SOURCE_DIR "${ASP_THERM_ROOT}/source/core")
set(THERMDB_SOURCE_DIR "${ASP_THERM_ROOT}/source/database")
set(THERMUTILS_SOURCE_DIR "${ASP_THERM_ROOT}/source/utils")

# setup file paths and cmake
#   error codes
set(DEFINES_SOURCE "")
set(ERRORCODES_FILE "merror_codes.h")
set(ERRORCODES_PATH "${THERMCORE_SOURCE_DIR}/common/${ERRORCODES_FILE}")
if(EXISTS ${ERRORCODES_PATH})
  message(STATUS "Add file with codes of errors: ${ERRORCODES_PATH}")
  # include(${ERRORCODES_PATH})
  set(INCLUDE_ERRORCODES TRUE)
  add_definitions(-DINCLUDE_ERRORCODES)
endif()

if(UNIX)
  add_definitions(-DOS_NIX)
  include_directories("${ASP_THERM_ROOT}/include/target_sys/_nix")
elseif(MSVC)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS -DOS_WIN)
  include_directories("${ASP_THERM_ROOT}/include/target_sys/win")
else()
  message(SEND_ERROR "UNDEFINED SYSTEM ${CMAKE_SYSTEM_NAME}")
endif(UNIX)
