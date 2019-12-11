################################################################################
### metadata ###################################################################
################################################################################

cmake_minimum_required(VERSION 3.5)

project(hmp VERSION "0.2.0" LANGUAGES C CXX)

include(ExternalProject)

################################################################################
### user config ################################################################
################################################################################

set(LLCE_DYLOAD ON CACHE BOOL "Enable dynamic loading of loop-live library.")
set(LLCE_FDOUBLE OFF CACHE BOOL "Enable double precision floating-point values.")
set(LLCE_CAPTURE OFF CACHE BOOL "Enable screen/state capture features (requires libpng).")

set(LLCE_MAX_RESOLUTION 1048576 CACHE STRING "The maximum resolution for application graphics buffers.")
set(LLCE_FPS 60 CACHE STRING "The target frames per second for the application.")
set(LLCE_SIMULATION demo CACHE STRING "The name of the hardness simulation (e.g. 'demo' for 'hmp/src/demo').")

# TODO(JRC): This is a hacky way of determining if CMake was configured to build
# in debug or release mode that should be replaced with something more robust.
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(LLCE_DEBUG ON CACHE BOOL "")
else()
    set(LLCE_DEBUG OFF CACHE BOOL "")
endif()

################################################################################
### compiler/linker ############################################################
################################################################################

set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y -Wall -Wno-unused-variable -fno-exceptions")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DGL_GLEXT_PROTOTYPES")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DGLM_ENABLE_EXPERIMENTAL -DGLM_FORCE_RADIANS -DGLM_FORCE_XYZW_ONLY")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -DLLCE_DEBUG=1")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DLLCE_DEBUG=0")

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--enable-new-dtags")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--enable-new-dtags -Wl,--export-dynamic")

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${PROJECT_BINARY_DIR}/install" CACHE PATH "Default install path." FORCE)
endif()

set(BUILD_SHARED_LIBS ON)
set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

################################################################################
### dependencies ###############################################################
################################################################################

find_package(OpenGL REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_search_module(SDL2 REQUIRED sdl2)
pkg_search_module(SDL2_ttf REQUIRED SDL2_ttf)
if(LLCE_CAPTURE)
    pkg_search_module(LIBPNG REQUIRED libpng16)
endif()

ExternalProject_Add(glm
    PREFIX ${PROJECT_BINARY_DIR}/glm_src
    URL https://github.com/g-truc/glm/releases/download/0.9.9.3/glm-0.9.9.3.zip
    URL_HASH MD5=d54fd62d29f690ad37683432921c3654
    DOWNLOAD_NAME "glm.zip"
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${PROJECT_BINARY_DIR}/glm_src/src/glm/glm ${PROJECT_BINARY_DIR}/glm/glm
    LOG_DOWNLOAD ON)
set(GLM_INCLUDE_DIRS ${PROJECT_BINARY_DIR}/glm)

################################################################################
### sources ####################################################################
################################################################################

set(sim_cnf_dir ${PROJECT_BINARY_DIR}/config)
file(GLOB_RECURSE sim_cnf_files ${PROJECT_SOURCE_DIR}/src/*.h*.in)
foreach(sim_cnf_file ${sim_cnf_files})
    get_filename_component(sim_cnf_filename ${sim_cnf_file} NAME)
    string(REGEX REPLACE "\.in" "" sim_cnf_basename ${sim_cnf_filename})
    configure_file(${sim_cnf_file} ${sim_cnf_dir}/${sim_cnf_basename})
endforeach()

################################################################################
### targets ####################################################################
################################################################################

set(sim_cnf_target ${PROJECT_NAME}.cnf CACHE INTERNAL "" FORCE)

add_library(${sim_cnf_target} INTERFACE)
add_dependencies(${sim_cnf_target} glm)
target_include_directories(${sim_cnf_target} INTERFACE ${sim_cnf_dir} ${GLM_INCLUDE_DIRS})

################################################################################
### testing ####################################################################
################################################################################



################################################################################
### packaging ##################################################################
################################################################################

install(DIRECTORY dat DESTINATION ${CMAKE_INSTALL_PREFIX})
install(DIRECTORY DESTINATION ${CMAKE_INSTALL_PREFIX}/out)

################################################################################
### sub-packaging ##############################################################
################################################################################

add_subdirectory("${PROJECT_SOURCE_DIR}/src")

################################################################################
### finalizing #################################################################
################################################################################

# Now comes everything we need, to create a package
# there are a lot more variables you can set, and some
# you need to set for some package types, but we want to
# be minimal here.
set(CPACK_PACKAGE_NAME "hmp")
set(CPACK_PACKAGE_VERSION "0.0.1")

# We don't want to split our program up into several incomplete pieces.
set(CPACK_MONOLITHIC_INSTALL TRUE)

# This must be last
include(CPack)
