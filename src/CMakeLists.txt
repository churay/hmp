################################################################################
### metadata ###################################################################
################################################################################



################################################################################
### user config ################################################################
################################################################################



################################################################################
### sources ####################################################################
################################################################################

# TODO(JRC): Clean up this hack that facilitates sharing of input handling code
# between the simulation and the steering components.
file(GLOB         hmp_shared_sources  ${CMAKE_CURRENT_SOURCE_DIR}/input.cpp)
file(GLOB         hmp_shared_headers  ${CMAKE_CURRENT_SOURCE_DIR}/input.h)

set(hmp_shared_dirs "")
foreach(hmp_shared_header ${hmp_shared_headers})
    set(hmp_shared_dir "")
    get_filename_component(hmp_shared_dir ${hmp_shared_header} DIRECTORY)
    list(APPEND hmp_shared_dirs ${hmp_shared_dir})
endforeach()

file(GLOB         hmp_plat_obj_sources  ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB         hmp_plat_obj_headers  ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
file(GLOB         hmp_plat_exe_sources  ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)
list(REMOVE_ITEM  hmp_plat_obj_sources  ${hmp_plat_exe_sources})

################################################################################
### targets ####################################################################
################################################################################

set(hmp_plat_obj_target "hmpobj")
set(hmp_plat_exe_target "hmp.out")

add_library(${hmp_plat_obj_target} OBJECT ${hmp_plat_obj_sources})
add_executable(${hmp_plat_exe_target} ${hmp_plat_exe_sources} $<TARGET_OBJECTS:${hmp_plat_obj_target}>)

foreach(hmp_target ${hmp_plat_obj_target} ${hmp_plat_exe_target})
    add_dependencies(${hmp_target} glm)
    target_include_directories(${hmp_target} PRIVATE ${hmp_cnf_dir} ${hmp_shared_dirs} ${CMAKE_CURRENT_SOURCE_DIR})
    target_include_directories(${hmp_target} PRIVATE ${SDL2_INCLUDE_DIRS} ${SDL2_ttf_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIR} ${GLM_INCLUDE_DIRS})
endforeach()
target_link_libraries(${hmp_plat_exe_target} PRIVATE ${SDL2_LIBRARIES} ${SDL2_ttf_LIBRARIES} ${OPENGL_LIBRARIES} ${CMAKE_DL_LIBS})

if(LLCE_CAPTURE)
    target_include_directories(${hmp_plat_exe_target} PRIVATE ${LIBPNG_INCLUDE_DIRS})
    target_link_libraries(${hmp_plat_exe_target} PRIVATE ${LIBPNG_LIBRARIES})
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

# All install commands get the same destination. this allows us to use paths
# relative to the executable.
install(TARGETS ${hmp_plat_exe_target} RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})

################################################################################
### sub-packaging ##############################################################
################################################################################

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/hmp")
