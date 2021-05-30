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

function(append_list_defines_system DEFINES_LIST)
    if(MSVC)
        message(STATUS "asp_therm add define: CRT_SECURE_NO_WARNINGS")
        list(APPEND DEFINES "-D_CRT_SECURE_NO_WARNINGS")
    endif()
    if(WIN32)
        message(STATUS "asp_therm add define: OS_WINDOWS")
        list(APPEND DEFINES "-DOS_WINDOWS")
    elseif(UNIX)
        message(STATUS "asp_therm add define: OS_UNIX")
        list(APPEND DEFINES "-DOS_UNIX")
    else()
        message(SEND_ERROR "Undefined system, please check CMakeList.txt")
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
    append_list_defines_options(DEFINES1)
    append_list_defines_system(DEFINES2)
    set(${DEFINES_LIST} ${DEFINES1} ${DEFINES1} PARENT_SCOPE)
endfunction()

function(post_build)
    if(UNIX)
        message(STATUS "\t\tGenerate compile_commands.json for asp_therm")
        # Генерировать compile_commands.json файл компилируемых объектов по которым ccls
        # будет правильно инклуды/дефайны расставлять
        set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
        # Скопировать compile_commands в корень проекта
        add_custom_target(
                copy-compile-commands-asp_therm ALL
                ${CMAKE_COMMAND} -E copy_if_different
                ${CMAKE_BINARY_DIR}/compile_commands.json ${CMAKE_CURRENT_SOURCE_DIR})
    endif(UNIX)
endfunction(post_build)
