cmake_minimum_required(VERSION 3.5)
project(hmp)

###############################################################################
## config options #############################################################
###############################################################################

option(BUILD_DEBUG "Build all source files in debug mode" ON)

###############################################################################
## file globbing ##############################################################
###############################################################################

set(hmp_exe_files src/hmp.cpp)
set(hmp_lib_files src/hmplib.cpp src/hmplib.h)

file(GLOB_RECURSE hmp_data_files    dat/*)
file(GLOB_RECURSE hmp_src_files     src/*.cpp src/*.h)
list(REMOVE_ITEM  hmp_src_files     ${hmp_exe_files} ${hmp_lib_files})

###############################################################################
## target definitions #########################################################
###############################################################################

set(hmp_cxx_flags    -std=c++1y -Wall -fno-exceptions)

add_library(hmplib SHARED ${hmp_lib_files} ${hmp_src_files} ${hmp_data_files})
target_compile_options(hmplib PUBLIC ${hmp_cxx_flags})
target_include_directories(hmplib PUBLIC src)

add_executable(hmp ${hmp_exe_files} ${hmp_src_files} ${hmp_data_files})
target_compile_options(hmp PUBLIC ${hmp_cxx_flags})
target_include_directories(hmp PUBLIC src)

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

include_directories(hmplib ${SDL2_INCLUDE_DIRS} ${SDL2_ttf_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIR})
target_link_libraries(hmplib PUBLIC ${SDL2_LIBRARIES} ${SDL2_ttf_LIBRARIES} ${OPENGL_LIBRARIES} ${CMAKE_DL_LIBS})

include_directories(hmp ${SDL2_INCLUDE_DIRS} ${SDL2_ttf_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIR})
target_link_libraries(hmp PUBLIC ${SDL2_LIBRARIES} ${SDL2_ttf_LIBRARIES} ${OPENGL_LIBRARIES} ${CMAKE_DL_LIBS})

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
install(TARGETS hmp hmplib DESTINATION hmp_install)

# This is basically a repeat of the file copy instruction that copies the
# resources in the build directory, but here we tell cmake that we want it
# in the package.
install(DIRECTORY hmp DESTINATION hmp_install)

# Now comes everything we need, to create a package
# there are a lot more variables you can set, and some
# you need to set for some package types, but we want to
# be minimal here.
set(CPACK_PACKAGE_NAME "hmp")
set(CPACK_PACKAGE_VERSION "0.0.a")

# We don't want to split our program up into several incomplete pieces.
set(CPACK_MONOLITHIC_INSTALL 1)

# This must be last
include(CPack)
