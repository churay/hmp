################################################################################
### metadata ###################################################################
################################################################################



################################################################################
### user config ################################################################
################################################################################



################################################################################
### sources ####################################################################
################################################################################

file(GLOB         demo_lib_sources     ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB         demo_lib_headers     ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
file(GLOB         demo_dat_sources     ${CMAKE_CURRENT_SOURCE_DIR}/demo_data*.c*)
file(GLOB         demo_dat_headers     ${CMAKE_CURRENT_SOURCE_DIR}/demo_data*.h*)
list(REMOVE_ITEM  demo_lib_sources     ${demo_dat_sources})
list(REMOVE_ITEM  demo_lib_headers     ${demo_dat_headers})

################################################################################
### targets ####################################################################
################################################################################

set(demo_dat_target demodata CACHE INTERNAL "" FORCE)
set(demo_lib_target demo CACHE INTERNAL "" FORCE)
if(LLCE_DYLOAD)
    set(demo_lib_type MODULE)
endif()

add_library(${demo_dat_target} ${demo_lib_type} ${demo_dat_sources})
add_library(${demo_lib_target} ${demo_lib_type} ${demo_lib_sources})
foreach(demo_target ${demo_dat_target} ${demo_lib_target})
    target_link_libraries(${demo_target} PUBLIC ${util_lib_target})
    target_include_directories(${demo_target} PRIVATE ${SDL2_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIR})
    target_link_libraries(${demo_target} PUBLIC ${SDL2_LIBRARIES} ${OPENGL_LIBRARIES})
endforeach()

if(NOT LLCE_DYLOAD)
    target_link_libraries(${demo_lib_target} PRIVATE ${demo_dat_target})
endif()

################################################################################
### packaging ##################################################################
################################################################################

# TODO(JRC): The solution below works when running CMake in serial because the
# "install.lock" file will necessarily be written before any changes are made
# to the project's install targets and be removed afterwards based on CMake's
# install script generation rules. It isn't clear whether or not these rules
# extend to parallel builds (though they probably do at least for Makefile outputs).

install(CODE "file(WRITE ${CMAKE_INSTALL_PREFIX}/install.lock \"\")")
install(TARGETS ${demo_dat_target} DESTINATION ${CMAKE_INSTALL_PREFIX})
install(TARGETS ${demo_lib_target} DESTINATION ${CMAKE_INSTALL_PREFIX})
install(CODE "file(REMOVE ${CMAKE_INSTALL_PREFIX}/install.lock)")

################################################################################
### sub-packaging ##############################################################
################################################################################


