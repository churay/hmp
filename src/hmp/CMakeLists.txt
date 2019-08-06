################################################################################
### metadata ###################################################################
################################################################################



################################################################################
### user config ################################################################
################################################################################



################################################################################
### sources ####################################################################
################################################################################

file(GLOB         hmp_sim_lib_sources     ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB         hmp_sim_lib_headers     ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
file(GLOB         hmp_sim_dat_sources     ${CMAKE_CURRENT_SOURCE_DIR}/hmp_data*.c*)
file(GLOB         hmp_sim_dat_headers     ${CMAKE_CURRENT_SOURCE_DIR}/hmp_data*.h*)
list(REMOVE_ITEM  hmp_sim_lib_sources     ${hmp_sim_dat_sources})
list(REMOVE_ITEM  hmp_sim_lib_headers     ${hmp_sim_dat_headers})

################################################################################
### targets ####################################################################
################################################################################

set(hmp_sim_dat_target "hmpdata")
set(hmp_sim_lib_target "hmp")

add_library(${hmp_sim_dat_target} ${hmp_sim_dat_sources} ${hmp_shared_sources})
add_library(${hmp_sim_lib_target} ${hmp_sim_lib_sources} ${hmp_shared_sources})
add_dependencies(${hmp_sim_lib_target} ${hmp_sim_dat_target})

foreach(hmp_target ${hmp_sim_dat_target} ${hmp_sim_lib_target})
    add_dependencies(${hmp_target} glm)
    target_include_directories(${hmp_target} PRIVATE ${hmp_cnf_dir} ${hmp_shared_dirs} ${CMAKE_CURRENT_SOURCE_DIR})
    target_include_directories(${hmp_target} PRIVATE ${SDL2_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIR} ${GLM_INCLUDE_DIRS})
    target_link_libraries(${hmp_target} PUBLIC ${SDL2_LIBRARIES} ${OPENGL_LIBRARIES})
endforeach()
target_link_libraries(${hmp_sim_lib_target} PRIVATE $<TARGET_FILE:${hmp_sim_dat_target}>)

################################################################################
### testing ####################################################################
################################################################################



################################################################################
### packaging ##################################################################
################################################################################

install(TARGETS ${hmp_sim_dat_target} LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX})
install(TARGETS ${hmp_sim_lib_target} LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX})
