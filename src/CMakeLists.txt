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

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/hmp")

file(GLOB         hmp_plat_obj_sources  ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB         hmp_plat_obj_headers  ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
file(GLOB         hmp_plat_exe_sources  ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)
list(REMOVE_ITEM  hmp_plat_obj_sources  ${hmp_plat_exe_sources})

################################################################################
### targets ####################################################################
################################################################################

# set(hmp_plat_obj_target "hmpobj")
# set(hmp_plat_exe_target "hmp.out")
# 
# add_library(${hmp_plat_obj_target} OBJECT ${hmp_plat_obj_sources})
# add_executable(${hmp_plat_exe_target} ${hmp_plat_exe_sources} $<TARGET_OBJECTS:${hmp_plat_obj_target}>)
# 
# foreach(hmp_target ${hmp_obj_target} ${hmp_dat_target} ${hmp_lib_target} ${hmp_exe_target})
#     target_include_directories(${hmp_target} PUBLIC src ${hmp_cnf_dir})
#     add_dependencies(${hmp_target} glm)
# endforeach()
# 
# add_dependencies(${hmp_lib_target} ${hmp_dat_target})
# 
# target_include_directories(${hmp_dat_target} PUBLIC src/hmp)
# target_include_directories(${hmp_lib_target} PUBLIC src/hmp)
# 
# if(NOT LLCE_DYLOAD)
#     target_include_directories(${hmp_exe_target} PUBLIC src/hmp)
#     add_dependencies(${hmp_exe_target} ${hmp_dat_target})
#     add_dependencies(${hmp_exe_target} ${hmp_lib_target})
# endif()
# 
# ################################################################################
# ### dependencies ###############################################################
# ################################################################################
# 
# set(hmp_dep_incs ${SDL2_INCLUDE_DIRS} ${SDL2_ttf_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIR} ${GLM_INCLUDE_DIRS})
# set(hmp_dep_libs ${SDL2_LIBRARIES} ${SDL2_ttf_LIBRARIES} ${OPENGL_LIBRARIES} ${CMAKE_DL_LIBS})
# 
# if(LLCE_CAPTURE)
#     pkg_search_module(LIBPNG REQUIRED libpng16)
#     set(hmp_exe_incs ${hmp_exe_incs} ${LIBPNG_INCLUDE_DIRS})
#     set(hmp_exe_libs ${hmp_exe_libs} ${LIBPNG_LIBRARIES})
# endif()
# 
# foreach(hmp_lib_target ${hmp_dat_target} ${hmp_lib_target})
#     target_include_directories(${hmp_lib_target} PUBLIC ${hmp_dep_incs})
#     target_link_libraries(${hmp_lib_target} PUBLIC ${hmp_dep_libs})
# endforeach()
# foreach(hmp_exe_target ${hmp_exe_target})
#     target_include_directories(${hmp_exe_target} PUBLIC ${hmp_dep_incs} ${hmp_exe_incs})
#     target_link_libraries(${hmp_exe_target} PUBLIC ${hmp_dep_libs} ${hmp_exe_libs})
# endforeach()
# target_link_libraries(${hmp_lib_target} PUBLIC $<TARGET_FILE:${hmp_dat_target}>)
# 
# if(NOT LLCE_DYLOAD)
#     target_link_libraries(${hmp_exe_target} PUBLIC $<TARGET_FILE:${hmp_dat_target}>)
#     target_link_libraries(${hmp_exe_target} PUBLIC $<TARGET_FILE:${hmp_lib_target}>)
# endif()
# 
# ################################################################################
# ### testing ####################################################################
# ################################################################################
# 
# # TODO(JRC): Add this feature once testing because useful for the project.
# 
# # # This is for our testing framework,
# # # we don't add REQUIRED because it's just for testing.
# # # People who might want to build the project to use it should not be required
# # # to install testing dependencies.
# # find_package(GTest)
# # 
# # if(GTEST_FOUND)
# #   add_executable(unit_tests ${sources_test} ${sources})
# # 
# #   # This define is added to prevent collision with the main.
# #   # It might be better solved by not adding the source with the main to the
# #   # testing target.
# #   target_compile_definitions(unit_tests PUBLIC UNIT_TESTS)
# # 
# #   # This allows us to use the executable as a link library, and inherit all 
# #   # linker options and library dependencies from it, by simply adding it as dependency.
# #   set_target_properties(example PROPERTIES ENABLE_EXPORTS on)
# # 
# #   target_link_libraries(unit_tests PUBLIC
# #     ${GTEST_BOTH_LIBRARIES}
# #     example
# #   )
# # 
# #   target_include_directories(unit_tests PUBLIC
# #     ${GTEST_INCLUDE_DIRS} # doesn't do anything on linux
# #   )
# #   
# # endif()
# 
# ################################################################################
# ### packaging ##################################################################
# ################################################################################
# 
# # TODO(JRC): The solution below works when running CMake in serial because the
# # "install.lock" file will necessarily be written before any changes are made
# # to the project's install targets and be removed afterwards based on CMake's
# # install script generation rules. It isn't clear whether or not these rules
# # extend to parallel builds (though they probably do at least for Makefile outputs).
# 
# # All install commands get the same destination. this allows us to use paths
# # relative to the executable.
# install(CODE "file(WRITE ${CMAKE_INSTALL_PREFIX}/install.lock \"\")")
# install(TARGETS ${hmp_dat_target} DESTINATION ${CMAKE_INSTALL_PREFIX})
# install(TARGETS ${hmp_lib_target} DESTINATION ${CMAKE_INSTALL_PREFIX})
# install(CODE "file(REMOVE ${CMAKE_INSTALL_PREFIX}/install.lock)")
# install(TARGETS ${hmp_exe_target} DESTINATION ${CMAKE_INSTALL_PREFIX})
# 
# install(DIRECTORY dat DESTINATION ${CMAKE_INSTALL_PREFIX})
# install(DIRECTORY DESTINATION ${CMAKE_INSTALL_PREFIX}/out)
# 
# # Now comes everything we need, to create a package
# # there are a lot more variables you can set, and some
# # you need to set for some package types, but we want to
# # be minimal here.
# set(CPACK_PACKAGE_NAME "hmp")
# set(CPACK_PACKAGE_VERSION "0.0.1")
# 
# # We don't want to split our program up into several incomplete pieces.
# set(CPACK_MONOLITHIC_INSTALL TRUE)
# 
# # This must be last
# include(CPack)
