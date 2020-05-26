macro(llce_install)
    set(options "")
    set(singleValueArgs PREFIX)
    set(multiValueArgs TARGETS)
    cmake_parse_arguments(arg "${options}" "${singleValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT DEFINED arg_TARGETS)
        message(FATAL_ERROR "::llce_install:: 'TARGETS' parameter must be non-empty list.")
    endif()
    if(NOT DEFINED arg_PREFIX)
        set(arg_PREFIX ${CMAKE_INSTALL_PREFIX})
    endif()

    # TODO(JRC): The solution below works when running CMake because the "install.lock"
    # file will necessarily exist while the install targets are being modified, which
    # allows for 'atomic' operations to occur based on the existence of this file.
    install(CODE "file(WRITE ${arg_PREFIX}/install.lock \"\")")
    install(TARGETS ${arg_TARGETS} DESTINATION ${arg_PREFIX})
    install(CODE "file(REMOVE ${arg_PREFIX}/install.lock)")
endmacro(llce_install)

macro(llce_simulation)
    set(options "")
    set(singleValueArgs NAME)
    set(multiValueArgs BASE_SOURCES DATA_SOURCES)
    cmake_parse_arguments(arg "${options}" "${singleValueArgs}" "${multiValueArgs}" ${ARGN})

    if("${arg_NAME}" STREQUAL "")
        message(FATAL_ERROR "::llce_simulation:: 'NAME' parameter must be non-empty string.")
    endif()
    if(NOT DEFINED arg_BASE_SOURCES)
        message(FATAL_ERROR "::llce_simulation:: 'BASE_SOURCES' parameter must be non-empty list.")
    endif()
    if(NOT DEFINED arg_DATA_SOURCES)
        message(FATAL_ERROR "::llce_simulation:: 'DATA_SOURCES' parameter must be non-empty list.")
    endif()

    if(BUILD_SHARED_LIBS)
        set(_sim_lib_type MODULE)
    endif()

    add_library(${arg_NAME}data ${_sim_lib_type} ${arg_DATA_SOURCES})
    add_library(${arg_NAME} ${_sim_lib_type} ${arg_BASE_SOURCES})
    foreach(_sim_target ${arg_NAME}data ${arg_NAME})
        target_include_directories(${_sim_target} PRIVATE ${SDL2_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIR})
        target_link_libraries(${_sim_target} PUBLIC llceutil ${SDL2_LIBRARIES} ${OPENGL_LIBRARIES})
    endforeach()

    if(NOT BUILD_SHARED_LIBS)
        target_link_libraries(${arg_NAME } PRIVATE ${arg_NAME}data)
    endif()

    llce_install(TARGETS ${arg_NAME}data ${arg_NAME})

    unset(_sim_lib_type)
    unset(_sim_target)
endmacro(llce_simulation)
