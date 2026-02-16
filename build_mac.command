#!/usr/bin/env bash
set -e

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
SCRIPT_NAME="build_unix.sh"

source "${SCRIPT_DIR}/${SCRIPT_NAME}" "$1"
