cmake_minimum_required(VERSION 3.5)
project(hmp)

include(ExternalProject)

###############################################################################
## config options #############################################################
###############################################################################

set(CMAKE_VERBOSE_MAKEFILE ON)

###########################################
## compiler/linker options ################
###########################################

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y -Wall -fno-exceptions")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DGL_GLEXT_PROTOTYPES")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DGLM_ENABLE_EXPERIMENTAL -DGLM_FORCE_RADIANS -DGLM_FORCE_XYZW_ONLY")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -DLLCE_DEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--enable-new-dtags")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--enable-new-dtags -Wl,--export-dynamic")

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set (CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "Default install path" FORCE)
endif()

####################################
## rpath options ###################
####################################

set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

####################################
## user options ####################
####################################

option(HMP_CAPTURE_ENABLED "Enable screen/state capture features (requires libpng/libffmpeg)." OFF)

###############################################################################
## file globbing ##############################################################
###############################################################################

file(GLOB         hmp_exe_files     src/main.cpp)
file(GLOB         hmp_obj_files     src/*.cpp src/*.h)
list(REMOVE_ITEM  hmp_obj_files     ${hmp_exe_files})
file(GLOB         hmp_lib_files     src/hmp/*.cpp src/hmp/*.h)
file(GLOB         hmp_dat_files     src/hmp/hmp_data*)
list(REMOVE_ITEM  hmp_lib_files     ${hmp_dat_files})

###############################################################################
## target definitions #########################################################
###############################################################################

####################################
## extra targets ###################
####################################

ExternalProject_Add(glm
    PREFIX ${CMAKE_BINARY_DIR}/glm_src
    URL https://github.com/g-truc/glm/releases/download/0.9.9.3/glm-0.9.9.3.zip
    URL_HASH MD5=d54fd62d29f690ad37683432921c3654
    DOWNLOAD_NAME "glm.zip"
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_BINARY_DIR}/glm_src/src/glm/glm ${CMAKE_BINARY_DIR}/glm/glm
    LOG_DOWNLOAD ON)
set(GLM_INCLUDE_DIRS ${CMAKE_BINARY_DIR}/glm)

####################################
## project targets #################
####################################

set(hmp_obj_target "hmpobj")
set(hmp_dat_target "hmpdata")
set(hmp_lib_target "hmp")
set(hmp_exe_target "hmp.out")

add_library(${hmp_obj_target} OBJECT ${hmp_obj_files})
add_library(${hmp_dat_target} SHARED ${hmp_dat_files})
add_library(${hmp_lib_target} SHARED ${hmp_lib_files} $<TARGET_OBJECTS:${hmp_obj_target}>)
add_executable(${hmp_exe_target} ${hmp_exe_files} $<TARGET_OBJECTS:${hmp_obj_target}>)

foreach(hmp_target ${hmp_obj_target} ${hmp_dat_target} ${hmp_lib_target})
    set_property(TARGET ${hmp_target} PROPERTY POSITION_INDEPENDENT_CODE ON)
endforeach()

foreach(hmp_target ${hmp_obj_target} ${hmp_dat_target} ${hmp_lib_target} ${hmp_exe_target})
    target_include_directories(${hmp_target} PUBLIC src)
    add_dependencies(${hmp_target} glm)
endforeach()

add_dependencies(${hmp_lib_target} ${hmp_dat_target})
# add_dependencies(${hmp_exe_target} ${hmp_lib_target})

target_include_directories(${hmp_dat_target} PUBLIC src/hmp)
target_include_directories(${hmp_lib_target} PUBLIC src/hmp)

###############################################################################
## dependencies ###############################################################
###############################################################################

find_package(OpenGL REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_search_module(SDL2 REQUIRED sdl2)
pkg_search_module(SDL2_ttf REQUIRED SDL2_ttf)

set(hmp_dep_incs ${SDL2_INCLUDE_DIRS} ${SDL2_ttf_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIR} ${GLM_INCLUDE_DIRS})
set(hmp_dep_libs ${SDL2_LIBRARIES} ${SDL2_ttf_LIBRARIES} ${OPENGL_LIBRARIES} ${CMAKE_DL_LIBS})

if(HMP_CAPTURE_ENABLED)
    pkg_search_module(LIBPNG REQUIRED libpng16)
    set(hmp_exe_incs ${hmp_exe_incs} ${LIBPNG_INCLUDE_DIRS})
    set(hmp_exe_libs ${hmp_exe_libs} ${LIBPNG_LIBRARIES})
endif()

foreach(hmp_lib_target ${hmp_dat_target} ${hmp_lib_target})
    target_include_directories(${hmp_lib_target} PUBLIC ${hmp_dep_incs})
    target_link_libraries(${hmp_lib_target} PUBLIC ${hmp_dep_libs})
endforeach()
foreach(hmp_exe_target ${hmp_exe_target})
    target_include_directories(${hmp_exe_target} PUBLIC ${hmp_dep_incs} ${hmp_exe_incs})
    target_link_libraries(${hmp_exe_target} PUBLIC ${hmp_dep_libs} ${hmp_exe_libs})
endforeach()
target_link_libraries(${hmp_lib_target} PUBLIC $<TARGET_FILE:${hmp_dat_target}>)
# target_link_libraries(${hmp_exe_target} PUBLIC $<TARGET_FILE:${hmp_lib_target}>)

###############################################################################
## testing ####################################################################
###############################################################################

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

###############################################################################
## packaging ##################################################################
###############################################################################

# TODO(JRC): The solution below works when running CMake in serial because the
# "install.lock" file will necessarily be written before any changes are made
# to the project's install targets and be removed afterwards based on CMake's
# install script generation rules. It isn't clear whether or not these rules
# extend to parallel builds (though they probably do at least for Makefile outputs).

# All install commands get the same destination. this allows us to use paths
# relative to the executable.
install(CODE "file(WRITE ${CMAKE_INSTALL_PREFIX}/install.lock \"\")")
install(TARGETS ${hmp_dat_target} DESTINATION ${CMAKE_INSTALL_PREFIX})
install(TARGETS ${hmp_lib_target} DESTINATION ${CMAKE_INSTALL_PREFIX})
install(CODE "file(REMOVE ${CMAKE_INSTALL_PREFIX}/install.lock)")
install(TARGETS ${hmp_exe_target} DESTINATION ${CMAKE_INSTALL_PREFIX})

install(DIRECTORY dat DESTINATION ${CMAKE_INSTALL_PREFIX})
install(DIRECTORY DESTINATION ${CMAKE_INSTALL_PREFIX}/out)

# Now comes everything we need, to create a package
# there are a lot more variables you can set, and some
# you need to set for some package types, but we want to
# be minimal here.
set(CPACK_PACKAGE_NAME "hmp")
set(CPACK_PACKAGE_VERSION "0.0.a")

# We don't want to split our program up into several incomplete pieces.
set(CPACK_MONOLITHIC_INSTALL TRUE)

# This must be last
include(CPack)
