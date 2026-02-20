#!/usr/bin/env bash
set -e

echo "Start building"
########### Defining script directory ###########
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

########### Build type ##########################
BUILD_TYPE="Debug"
ENABLE_CHEATS=OFF

if [[ "$1" == "release" || "$1" == "Release" ]]; then
    BUILD_TYPE="Release"
elif [[ "$1" == "debug" || "$1" == "Debug" || -z "$1" ]]; then
    BUILD_TYPE="Debug"
    ENABLE_CHEATS=ON

else
    echo "Unknown build type: $1"
    echo "Usage: $0 [debug|release]"
    exit 1
fi

########### Defining exec filepath #############
EXEC_FILE="${SCRIPT_DIR}"/build/${BUILD_TYPE}/game_engine

########### Configure ##########################
echo "Configuration: ${BUILD_TYPE}"
cmake -S "${SCRIPT_DIR}" \
      -B "${SCRIPT_DIR}"/build \
      -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
      -DENABLE_CHEATS="${ENABLE_CHEATS}"

########### Build ##############################
cmake --build "${SCRIPT_DIR}"/build -j 8

echo "Build finished"

########### Run #################################
"${EXEC_FILE}"
