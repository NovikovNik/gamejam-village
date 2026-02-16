#!/usr/bin/env bash
set -e

echo "Start building"
########### Defining script directory ###########
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
########### Defining exec directory #############
EXEC_FILE="${SCRIPT_DIR}"/build/game_engine

########### Build type ##########################
BUILD_TYPE="Debug"

if [[ "$1" == "release" || "$1" == "Release" ]]; then
    BUILD_TYPE="Release"
elif [[ "$1" == "debug" || "$1" == "Debug" || -z "$1" ]]; then
    BUILD_TYPE="Debug"
else
    echo "Unknown build type: $1"
    echo "Usage: $0 [debug|release]"
    exit 1
fi

########### Configure ##########################
echo "Configuration: ${BUILD_TYPE}"
cmake -S "${SCRIPT_DIR}" \
      -B "${SCRIPT_DIR}"/build \
      -DCMAKE_BUILD_TYPE="${BUILD_TYPE}"

########### Build ##############################
cmake --build "${SCRIPT_DIR}"/build

echo "Build finished"

########### Run #################################
"${EXEC_FILE}"
