cmake_minimum_required(VERSION 3.5)
project(hmp)

###############################################################################
## config options #############################################################
###############################################################################

# NOTE(JRC): Adding a debug build option is unnecessary because CMake handles
# it with these commands:
# -DCMAKE_BUILD_TYPE=Release, -DCMAKE_BUILD_TYPE=Debug
# option(BUILD_DEBUG "Build all source files in debug mode" ON)

# set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y -Wall -fno-exceptions")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -DLLCE_DEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set (CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "Default install path" FORCE )
endif()

# TODO(JRC): Clean up this code that sets the RPATH for the project binary
# so that it can find the project dynamic library.

# use, i.e. don't skip the full RPATH for the build tree
SET(CMAKE_SKIP_BUILD_RPATH  FALSE)
# when building, don't use the install RPATH already
# (but later on when installing)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}")
# add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

###############################################################################
## file globbing ##############################################################
###############################################################################

file(GLOB_RECURSE hmp_src_files     src/*.cpp src/*.h)
file(GLOB_RECURSE hmp_obj_files     src/*.cpp)
file(GLOB         hmp_exe_files     src/hmp.cpp)
file(GLOB         hmp_lib_files     src/hmplib.cpp src/hmplib.h)
list(REMOVE_ITEM  hmp_src_files     ${hmp_exe_files} ${hmp_lib_files})
list(REMOVE_ITEM  hmp_obj_files     ${hmp_exe_files} ${hmp_lib_files})

###############################################################################
## target definitions #########################################################
###############################################################################

set(hmp_obj_target "hmpobj")
set(hmp_lib_target "hmplib")
set(hmp_exe_target "hmp")
set(hmp_targets ${hmp_lib_target} ${hmp_exe_target})

add_library(${hmp_obj_target} OBJECT ${hmp_obj_files})
set_property(TARGET ${hmp_obj_target} PROPERTY POSITION_INDEPENDENT_CODE ON)
add_library(${hmp_lib_target} SHARED ${hmp_lib_files} $<TARGET_OBJECTS:${hmp_obj_target}>)
add_executable(${hmp_exe_target} ${hmp_exe_files} $<TARGET_OBJECTS:${hmp_obj_target}>)

foreach(hmp_target ${hmp_obj_target} ${hmp_lib_target} ${hmp_exe_target})
    target_include_directories(${hmp_target} PUBLIC src)
endforeach()

# This copies all resource files in the build directory.
# We need this, because we want to work with paths relative to the executable.
file(COPY ${data} DESTINATION dat)

###############################################################################
## dependencies ###############################################################
###############################################################################

find_package(OpenGL REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_search_module(SDL2 REQUIRED sdl2)
pkg_search_module(SDL2_ttf REQUIRED SDL2_ttf)

set(hmp_dep_incs ${SDL2_INCLUDE_DIRS} ${SDL2_ttf_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIR})
set(hmp_dep_libs ${SDL2_LIBRARIES} ${SDL2_ttf_LIBRARIES} ${OPENGL_LIBRARIES} ${CMAKE_DL_LIBS})

foreach(hmp_target ${hmp_targets})
    target_include_directories(${hmp_target} PUBLIC ${hmp_dep_incs})
    target_link_libraries(${hmp_target} PUBLIC ${hmp_dep_libs})
endforeach()

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

# All install commands get the same destination. this allows us to use paths
# relative to the executable.
install(TARGETS ${hmp_targets} DESTINATION ${CMAKE_INSTALL_PREFIX})
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
