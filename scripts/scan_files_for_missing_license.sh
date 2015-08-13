#!/usr/bin/env bash

SCRIPT_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )

for arg in ${@} ; do
    find ${arg} -type f -a \( -name "*.cpp" -o -name "*.h" -o -name "*.hpp" -o -name "*.ypp" -o -name "*.re" \) -exec ${SCRIPT_DIR}/update_license.sh ${SCRIPT_DIR}/../LICENSE '{}' \;
done