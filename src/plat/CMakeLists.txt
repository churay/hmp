################################################################################
### metadata ###################################################################
################################################################################



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
### testing ####################################################################
################################################################################

# TODO(JRC): Add this feature once testing because useful for the project.

# # This is for our testing framework,
# # we don't add REQUIRED because it's just for testing.
# # People who might want to build the project to use it should not be required
# # to install testing dependencies.
# find_package(GTest)
# 
# if(GTEST_FOUND)
#   add_executable(unit_tests ${sources_test} ${sources})
# 
#   # This define is added to prevent collision with the main.
#   # It might be better solved by not adding the source with the main to the
#   # testing target.
#   target_compile_definitions(unit_tests PUBLIC UNIT_TESTS)
# 
#   # This allows us to use the executable as a link library, and inherit all 
#   # linker options and library dependencies from it, by simply adding it as dependency.
#   set_target_properties(example PROPERTIES ENABLE_EXPORTS on)
# 
#   target_link_libraries(unit_tests PUBLIC
#     ${GTEST_BOTH_LIBRARIES}
#     example
#   )
# 
#   target_include_directories(unit_tests PUBLIC
#     ${GTEST_INCLUDE_DIRS} # doesn't do anything on linux
#   )
#   
# endif()

################################################################################
### packaging ##################################################################
################################################################################

install(TARGETS ${plat_lib_target} DESTINATION ${CMAKE_INSTALL_PREFIX})

################################################################################
### sub-packaging ##############################################################
################################################################################


