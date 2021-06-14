function(append_list_defines_options DEFINES_LIST)
    # add pq, pqxx
    if(WITH_POSTGRESQL)
        list(APPEND DEFINES "-DBYCMAKE_WITH_POSTGRESQL")
    endif()
    if(WITH_FIREBIRD)
        list(APPEND DEFINES "-DBYCMAKE_WITH_FIREBIRD")
    endif()
    if(ISO_20765)
        list(APPEND DEFINES "-DBYCMAKE_ISO_20765")
    endif()
    set(${DEFINES_LIST} ${DEFINES} PARENT_SCOPE)
endfunction()

function(set_include_dirs INCLUDE_DIRS)
    set(THERMCORE_INCLUDES
            "${THERMCORE_SOURCE_DIR}/common" "${THERMCORE_SOURCE_DIR}/gas_parameters"
            "${THERMCORE_SOURCE_DIR}/models" "${THERMCORE_SOURCE_DIR}/phase_diagram"
            "${THERMCORE_SOURCE_DIR}/service" "${THERMCORE_SOURCE_DIR}/subroutins")

    set(${INCLUDE_DIRS} ${THERMCORE_INCLUDES} ${THERMDB_SOURCE_DIR} PARENT_SCOPE)
endfunction()

function(append_list_defines DEFINES_LIST)
    append_list_defines_options(DEFINES_OPT)
    append_list_defines_system(DEFINES_SYS)
    set(${DEFINES_LIST} ${DEFINES_OPT} ${DEFINES_SYS} PARENT_SCOPE)
endfunction()

