#!/usr/bin/env bash

script_path="${PWD}/test"
. ${script_path}/helpers.sh

test="xml"

mkdir "${test}"
pushd "${test}" >/dev/null 2>&1

    ffmpeg -y -f lavfi -i testsrc2=duration=1:size=720x480 -pix_fmt yuv411p -c:v dvvideo -aspect 4/3 test.dv >/dev/null 2>&1 || fatal "internal" "ffmpeg command failed"

    # check xml output
    run_dvrescue test.dv
    check_success
    if [ "${?}" -ne 0 ] ; then
        error "simple/xml" "command failed"
    fi

    check_xml
    if [ "${?}" -ne 0 ] ; then
        error "simple/xml" "invalid xml"
    fi

    # check xml output to file
    run_dvrescue test.dv --xml-output test.xml
    check_success
    if [ "${?}" -ne 0 ] ; then
        error "simple/xml" "command failed"
    fi

    if [ ! -e "test.xml" ] ; then
        error "$test" "xml output file not created"
    elif [ ! -s "test.xml" ] ; then
        error "$test" "xml output file is empty"
    else
        cmd_stdout="$(<test.xml)"
        check_xml
    fi

popd >/dev/null 2>&1
rm -fr "${test}"

exit ${status}
