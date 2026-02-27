#!/usr/bin/env bash
set -e

echo "Start building"
########### Defining script directory ###########
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

########### Build type ##########################
BUILD_TYPE="Debug"
ENABLE_CHEATS=OFF
# Имя исполняемого файла можно передать вторым аргументом, по умолчанию AAAB
EXEC_FILE_NAME="AAAB"

if [[ "$1" == "release" || "$1" == "Release" ]]; then
    BUILD_TYPE="Release"
elif [[ "$1" == "debug" || "$1" == "Debug" || -z "$1" ]]; then
    BUILD_TYPE="Debug"
    ENABLE_CHEATS=ON


else
    echo "Unknown build type: $1"
    echo "Usage: $0 [debug|release] [exec_name]"
    exit 1
fi

# Переопределяем имя исполняемого файла, если передан второй аргумент
if [[ -n "$2" ]]; then
    EXEC_FILE_NAME="$2"
fi

########### Defining exec filepath #############
EXEC_FILE="${SCRIPT_DIR}"/build/${BUILD_TYPE}/${EXEC_FILE_NAME}

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
if [[ "$(uname)" == "Darwin" ]]; then
    # На macOS открываем через open -a (например, если это .app bundle)
    open -a "${EXEC_FILE}"
else
    "${EXEC_FILE}"
fi
