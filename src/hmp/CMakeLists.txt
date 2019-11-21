################################################################################
### metadata ###################################################################
################################################################################



################################################################################
### user config ################################################################
################################################################################



################################################################################
### sources ####################################################################
################################################################################

file(GLOB         hmp_lib_sources     ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB         hmp_lib_headers     ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
file(GLOB         hmp_dat_sources     ${CMAKE_CURRENT_SOURCE_DIR}/hmp_data*.c*)
file(GLOB         hmp_dat_headers     ${CMAKE_CURRENT_SOURCE_DIR}/hmp_data*.h*)
list(REMOVE_ITEM  hmp_lib_sources     ${hmp_dat_sources})
list(REMOVE_ITEM  hmp_lib_headers     ${hmp_dat_headers})

################################################################################
### targets ####################################################################
################################################################################

set(hmp_dat_target hmpdata CACHE INTERNAL "" FORCE)
set(hmp_lib_target hmp CACHE INTERNAL "" FORCE)
if(LLCE_DYLOAD)
    set(hmp_lib_type MODULE)
else()
    set(hmp_lib_type STATIC)
endif()

add_library(${hmp_dat_target} ${hmp_lib_type} ${hmp_dat_sources})
add_library(${hmp_lib_target} ${hmp_lib_type} ${hmp_lib_sources})
add_dependencies(${hmp_lib_target} ${hmp_dat_target})

foreach(hmp_target ${hmp_dat_target} ${hmp_lib_target})
    add_dependencies(${hmp_target} glm ${sim_cnf_target} ${plat_lib_target})
    target_include_directories(${hmp_target} PRIVATE ${hmp_cnf_dir} ${hmp_shared_dirs} ${CMAKE_CURRENT_SOURCE_DIR})
    target_include_directories(${hmp_target} PRIVATE ${SDL2_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIR} ${GLM_INCLUDE_DIRS})
    target_link_libraries(${hmp_target} PUBLIC ${SDL2_LIBRARIES} ${OPENGL_LIBRARIES})
endforeach()
target_link_libraries(${hmp_lib_target} PRIVATE $<TARGET_FILE:${hmp_dat_target}>)

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

# TODO(JRC): The solution below works when running CMake in serial because the
# "install.lock" file will necessarily be written before any changes are made
# to the project's install targets and be removed afterwards based on CMake's
# install script generation rules. It isn't clear whether or not these rules
# extend to parallel builds (though they probably do at least for Makefile outputs).

install(CODE "file(WRITE ${CMAKE_INSTALL_PREFIX}/install.lock \"\")")
install(TARGETS ${hmp_dat_target} DESTINATION ${CMAKE_INSTALL_PREFIX})
install(TARGETS ${hmp_lib_target} DESTINATION ${CMAKE_INSTALL_PREFIX})
install(CODE "file(REMOVE ${CMAKE_INSTALL_PREFIX}/install.lock)")

################################################################################
### sub-packaging ##############################################################
################################################################################


