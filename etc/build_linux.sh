#!/bin/bash

set -v; set -x

# This script launches 'cmake' to build the project for Linux platforms.

SCRIPT_PATH="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null && pwd)"
PROJ_PATH=$(dirname ${SCRIPT_PATH})

BUILD_TYPE=${1:-Debug}

pushd .
cd ${PROJ_PATH}

# rm -rf build
mkdir -p build
cd build

cmake -DCMAKE_C_COMPILER=/usr/bin/clang -DCMAKE_CXX_COMPILER=/usr/bin/clang++ -DCMAKE_BUILD_TYPE=${BUILD_TYPE} ${PROJ_PATH}
make
make install

if [[ ! -f ${PROJ_PATH}/hmp ]]; then
    ln -s ${PROJ_PATH}/build/install/hmp ${PROJ_PATH}/hmp
fi

popd
