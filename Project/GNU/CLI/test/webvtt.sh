#!/usr/bin/env bash

script_path="${PWD}/test"
. ${script_path}/helpers.sh

test="webvtt"

mkdir "${test}"
pushd "${test}" >/dev/null 2>&1

    ffmpeg -y -f lavfi -i testsrc2=duration=1:size=720x480 -pix_fmt yuv411p -c:v dvvideo -aspect 4/3 test.dv >/dev/null 2>&1 || fatal "internal" "ffmpeg command failed"

    run_dvrescue test.dv --webvtt-output test.vtt
    check_success
    check_xml

    if [ ! -e "test.vtt" ] ; then
        error "$test" "WebVTT output file not created"
    elif [ ! -s "test.vtt" ] ; then
        error "$test" "WebVTT output file is empty"
    elif [ "$(head -c 6 test.vtt)" != "WEBVTT" ] ; then
        error "$test" "Invalid WebVTT output"
    fi

popd >/dev/null 2>&1
rm -fr "${test}"

exit ${status}
