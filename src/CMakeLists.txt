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
target_link_libraries(llcesim PRIVATE llceutil llceplat $<$<BOOL:LLCE_DEBUG>:llcemeta>)
target_include_directories(llcesim PRIVATE ${SDL2_INCLUDE_DIRS} ${SDL2_ttf_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIR})
target_link_libraries(llcesim PRIVATE ${SDL2_LIBRARIES} ${SDL2_ttf_LIBRARIES} ${OPENGL_LIBRARIES} ${CMAKE_DL_LIBS})

target_include_directories(llcesim PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/${LLCE_SIMULATION}")
# TODO(JRC): Maybe just 'target_link_libraries' will work because sim is a module.
if(LLCE_DYLOAD)
    add_dependencies(llcesim ${LLCE_SIMULATION}data ${LLCE_SIMULATION})
else()
    target_link_libraries(llcesim PRIVATE ${LLCE_SIMULATION}data ${LLCE_SIMULATION})
endif()

################################################################################
### packaging ##################################################################
################################################################################

install(TARGETS llcesim DESTINATION ${CMAKE_INSTALL_PREFIX})

################################################################################
### sub-packaging ##############################################################
################################################################################


