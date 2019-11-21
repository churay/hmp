################################################################################
### metadata ###################################################################
################################################################################



################################################################################
### user config ################################################################
################################################################################



################################################################################
### sources ####################################################################
################################################################################

file(GLOB sim_exe_sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB sim_exe_headers ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

set(sim_exe_subdirs util plat hmp)
foreach(sim_exe_subdir ${sim_exe_subdirs})
    add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/${sim_exe_subdir}")
endforeach()

################################################################################
### targets ####################################################################
################################################################################

set(sim_exe_target ${PROJECT_NAME}.out CACHE INTERNAL "" FORCE)

add_executable(${sim_exe_target} ${sim_exe_sources})
target_link_libraries(${sim_exe_target} PRIVATE ${util_lib_target} ${plat_lib_target})
target_include_directories(${sim_exe_target} PRIVATE ${SDL2_INCLUDE_DIRS} ${SDL2_ttf_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIR})
target_link_libraries(${sim_exe_target} PRIVATE ${SDL2_LIBRARIES} ${SDL2_ttf_LIBRARIES} ${OPENGL_LIBRARIES} ${CMAKE_DL_LIBS})

target_include_directories(${sim_exe_target} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/hmp")
if(LLCE_DYLOAD)
    add_dependencies(${sim_exe_target} ${hmp_dat_target} ${hmp_lib_target})
else()
    target_link_libraries(${sim_exe_target} PRIVATE ${hmp_dat_target} ${hmp_lib_target})
endif()

################################################################################
### packaging ##################################################################
################################################################################

install(TARGETS ${sim_exe_target} DESTINATION ${CMAKE_INSTALL_PREFIX})

################################################################################
### sub-packaging ##############################################################
################################################################################


