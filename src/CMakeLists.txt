################################################################################
### metadata ###################################################################
################################################################################



################################################################################
### user config ################################################################
################################################################################



################################################################################
### sources ####################################################################
################################################################################

file(GLOB llce_sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB llce_headers ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

################################################################################
### targets ####################################################################
################################################################################

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/util")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/plat")
if(LLCE_DEBUG)
    add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/meta")
endif()
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/${LLCE_SIMULATION}")

add_executable(llcesim ${llce_sources})
target_link_libraries(llcesim PRIVATE llceutil llceplat $<$<BOOL:${LLCE_DEBUG}>:llcemeta>)
target_include_directories(llcesim PRIVATE ${SDL2_INCLUDE_DIRS} ${SDL2_ttf_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIR})
target_link_libraries(llcesim PRIVATE ${SDL2_LIBRARIES} ${SDL2_ttf_LIBRARIES} ${OPENGL_LIBRARIES} ${CMAKE_DL_LIBS})

target_include_directories(llcesim PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/${LLCE_SIMULATION})
if(LLCE_DYLOAD)
    add_dependencies(llcesim ${LLCE_SIMULATION}data ${LLCE_SIMULATION})
else()
    target_link_libraries(llcesim PRIVATE ${LLCE_SIMULATION}data ${LLCE_SIMULATION})
endif()

# FIXME(JRC): The following command explicitly adds the container application's
# header file to the harness' object file dependencies, which will ensure that the
# harness is recompiled when the container's header changes. This is not an ideal
# solution, but my other tries using 'add_custom_command' and 'add_custom_target'
# didn't work.
set_property(SOURCE main.cpp APPEND PROPERTY OBJECT_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${LLCE_SIMULATION}/${LLCE_SIMULATION}.h)

################################################################################
### packaging ##################################################################
################################################################################

install(TARGETS llcesim DESTINATION ${CMAKE_INSTALL_PREFIX})

################################################################################
### sub-packaging ##############################################################
################################################################################


