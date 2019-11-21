################################################################################
### metadata ###################################################################
################################################################################

# NOTE(JRC): This submodule is dedicated to holding all of the source code that's
# used by the harness to wrangle platform-specific interactions. Ideally, all code
# in this submodule should be shielded from the embedded simulation.

################################################################################
### user config ################################################################
################################################################################



################################################################################
### sources ####################################################################
################################################################################

file(GLOB  plat_lib_sources  ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB  plat_lib_headers  ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

################################################################################
### targets ####################################################################
################################################################################

set(plat_lib_target llceplat CACHE INTERNAL "" FORCE)

add_library(${plat_lib_target} ${plat_lib_sources})
target_include_directories(${plat_lib_target} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${plat_lib_target} PUBLIC ${sim_cnf_target})
if(LLCE_CAPTURE)
    target_include_directories(${plat_lib_target} PRIVATE ${LIBPNG_INCLUDE_DIRS})
    target_link_libraries(${plat_lib_target} PUBLIC ${LIBPNG_LIBRARIES})
endif()

################################################################################
### packaging ##################################################################
################################################################################

install(TARGETS ${plat_lib_target} DESTINATION ${CMAKE_INSTALL_PREFIX})

################################################################################
### sub-packaging ##############################################################
################################################################################

