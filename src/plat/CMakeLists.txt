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

file(GLOB plat_sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB plat_headers ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

################################################################################
### targets ####################################################################
################################################################################

add_library(llceplat ${plat_sources})
target_include_directories(llceplat PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(llceplat PUBLIC llceconfig)
if(LLCE_CAPTURE)
    target_include_directories(llceplat PRIVATE ${LIBPNG_INCLUDE_DIRS})
    target_link_libraries(llceplat PUBLIC ${LIBPNG_LIBRARIES})
endif()

################################################################################
### packaging ##################################################################
################################################################################

install(TARGETS llceplat DESTINATION ${CMAKE_INSTALL_PREFIX})

################################################################################
### sub-packaging ##############################################################
################################################################################


