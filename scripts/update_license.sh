#!/usr/bin/env bash

# If this is not a regular file, exit.
if [[ ! -f $2 ]]; then
    exit 0
fi

# update_license
grep -q " * The MIT License (MIT)" $2

if [[ $? == 0 ]]; then
    exit 0
else
    echo "Prepending license to '$2'"
    tmpname=/dev/shm/update_license_tmp_$(date +%s)
    cat ${1} ${2} > ${tmpname} && mv -f ${tmpname} ${2}
fi

exit 0

