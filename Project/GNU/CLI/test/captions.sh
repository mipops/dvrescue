#!/usr/bin/env bash

script_path="${PWD}/test"
. ${script_path}/helpers.sh

test="captions"

mkdir "${test}"
pushd "${test}" >/dev/null 2>&1
    while read line ; do
        file="$(echo "${line}" | cut -d' ' -f1)"
        options="$(echo "${line}" | cut -d' ' -f2)"
        outputs="$(echo "${line}" | cut -d' ' -f3-)"

        if [ ! -e "${files_path}/${file}" ] ; then
            echo "NOK: ${test}/${file}[${options}], file not found" >&${fd}
            status=1
            continue
        fi

        run_dvrescue ${files_path}/${file} ${options//;/ }
        check_success

        for entry in ${outputs} ; do
            name="$(echo "${entry}" | cut -d';' -f1)"
            hash="$(echo "${entry}" | cut -d';' -f2)"

            if [ ! -e "${name}" ] ; then
                echo "NOK: ${test}/${file}[${options}], file ${name} not genetated" >&${fd}
                status=1
                continue
            fi

            if [ "$(${md5cmd} "${name}" | cut -d' ' -f1)" != "${hash}" ] ; then
                echo "NOK: ${test}/${file}[${options}], invalid file ${name}" >&${fd}
                status=1
                continue
            fi
        done
    done < "${script_path}/${test}.txt"
    
popd >/dev/null 2>&1
rm -fr "${test}"
exit ${status}
