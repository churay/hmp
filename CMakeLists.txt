################################################################################
### metadata ###################################################################
################################################################################

cmake_minimum_required(VERSION 3.5)

project(hmp VERSION "0.2.0" LANGUAGES C CXX)

include(ExternalProject)

################################################################################
### user config ################################################################
################################################################################

option(LLCE_DYLOAD "Enable dynamic loading of loop-live library." ON)
option(LLCE_FDOUBLE "Enable double precision floating-point values." OFF)
option(LLCE_CAPTURE "Enable screen/state capture features (requires libpng/libffmpeg)." OFF)
set(LLCE_MAX_RESOLUTION "1048576" CACHE STRING "The maximum resolution for application graphics buffers.")

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
set(BUILD_SHARED_LIBS ${LLCE_DYLOAD})

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${PROJECT_BINARY_DIR}/install" CACHE PATH "Default install path." FORCE)
endif()

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

set(hmp_cnf_dir ${PROJECT_BINARY_DIR}/config)
file(GLOB_RECURSE hmp_cnf_files ${PROJECT_SOURCE_DIR}/src/*.h.in)
foreach(hmp_cnf_file ${hmp_cnf_files})
    get_filename_component(hmp_cnf_filename ${hmp_cnf_file} NAME_WE)
    configure_file(${hmp_cnf_file} ${hmp_cnf_dir}/${hmp_cnf_filename}.h)
endforeach()

################################################################################
### targets ####################################################################
################################################################################



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
