#!/bin/bash

# This script launches 'cmake' to build the project for Linux platforms.

BUILD_TYPE=${1:-Debug}
CONFIG_TYPE=${2:-Default}
SIM_TYPE=${3:-hmp}
BUILD_TASKS=$(lscpu | sed -r -n 's/(^CPU[(]s[)]:.*([0-9]+).*$)/\2/p')

SCRIPT_PATH="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null && pwd)"
PROJ_PATH=$(dirname ${SCRIPT_PATH})

pushd . &> /dev/null
cd ${PROJ_PATH}

if [[ ${CONFIG_TYPE} != Default ]]; then rm -rf build; fi
mkdir -p build
cd build

cmake -DCMAKE_C_COMPILER=/usr/bin/clang -DCMAKE_CXX_COMPILER=/usr/bin/clang++ -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
    -DLLCE_DYLOAD=ON \
    -DLLCE_FDOUBLE=ON \
    -DLLCE_CAPTURE=ON \
    -DLLCE_SIMULATION=${SIM_TYPE} \
    ${PROJ_PATH}
make -j${BUILD_TASKS} && make -j${BUILD_TASKS} install

rm -f ${PROJ_PATH}/llce.out
ln -s ${PROJ_PATH}/build/install/llcesim ${PROJ_PATH}/llce.out

popd &> /dev/null
