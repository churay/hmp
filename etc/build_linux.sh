#!/bin/bash

# This script launches 'cmake' to build the project for Linux platforms.

BUILD_TYPE=${1:-Debug}

SCRIPT_PATH="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null && pwd)"
PROJ_PATH=$(dirname ${SCRIPT_PATH})

pushd .
cd ${PROJ_PATH}

# rm -rf build
mkdir -p build
cd build

cmake -DCMAKE_C_COMPILER=/usr/bin/clang -DCMAKE_CXX_COMPILER=/usr/bin/clang++ -DCMAKE_BUILD_TYPE=${BUILD_TYPE} ${PROJ_PATH}
make
make install

if [[ ! -f ${PROJ_PATH}/hmp.out ]]; then
    ln -s ${PROJ_PATH}/build/install/hmp.out ${PROJ_PATH}/hmp.out
fi

popd
