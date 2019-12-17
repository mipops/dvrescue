#!/usr/bin/env bash

script_path="${PWD}/test"
. ${script_path}/helpers.sh

test="frame_discontinuity"

mkdir "${test}"
pushd "${test}" >/dev/null 2>&1
    wget https://github.com/angus-c/angus-c.github.com/raw/master/fonts/andale-mono.ttf >/dev/null 2>&1 || fatal "internal" "wget command failed"

    ffmpeg -y -f lavfi -i testsrc2=r=30000/1001:s=720x480 -f lavfi -i sine -pix_fmt yuv411p -c:v dvvideo -aspect 4/3  -t 0.2 -ac 2 -ar 48000 -vf "drawtext=fontfile='andale-mono.ttf':x=(w-text_w)/2:y=(h-text_h)/2:fontsize=36:fontcolor=white:box=1:boxcolor=gray@0.6:boxborderw=12:shadowx=2:shadowy=2:text='DV25 NTSC 4\:1\:1 4\3  2Ch 48kHz'" dv25_ntsc_411_4-3_2ch_48k_bars_sine.dv >/dev/null 2>&1 || fatal "internal" "ffmpeg command failed"
    ffmpeg -y -f lavfi -i testsrc2=r=30000/1001:s=720x480 -f lavfi -i sine -pix_fmt yuv411p -c:v dvvideo -aspect 16/9 -t 0.2 -ac 2 -ar 48000 -vf "drawtext=fontfile='andale-mono.ttf':x=(w-text_w)/2:y=(h-text_h)/2:fontsize=36:fontcolor=white:box=1:boxcolor=gray@0.6:boxborderw=12:shadowx=2:shadowy=2:text='DV25 NTSC 4\:1\:1 16\9 2Ch 48kHz'" dv25_ntsc_411_169_2ch_48k_bars_sine.dv >/dev/null 2>&1 || fatal "internal" "ffmpeg command failed"
    ffmpeg -y -f lavfi -i testsrc2=r=25:s=720x576 -f lavfi -i sine -pix_fmt yuv411p -c:v dvvideo -aspect 4/3  -t 0.2 -ac 2 -ar 48000 -vf "drawtext=fontfile='andale-mono.ttf':x=(w-text_w)/2:y=(h-text_h)/2:fontsize=36:fontcolor=white:box=1:boxcolor=gray@0.6:boxborderw=12:shadowx=2:shadowy=2:text='DV25 PAL  4\:1\:1 4\3  2Ch 48kHz'" dv25_pal__411_4-3_2ch_48k_bars_sine.dv >/dev/null 2>&1 || fatal "internal" "ffmpeg command failed"
    ffmpeg -y -f lavfi -i testsrc2=r=25:s=720x576 -f lavfi -i sine -pix_fmt yuv411p -c:v dvvideo -aspect 4/3  -t 0.2 -ac 2 -ar 32000 -vf "drawtext=fontfile='andale-mono.ttf':x=(w-text_w)/2:y=(h-text_h)/2:fontsize=36:fontcolor=white:box=1:boxcolor=gray@0.6:boxborderw=12:shadowx=2:shadowy=2:text='DV25 PAL  4\:1\:1 4\3  2Ch 32kHz'" dv25_pal__411_4-3_2ch_32k_bars_sine.dv >/dev/null 2>&1 || fatal "internal" "ffmpeg command failed"
    ffmpeg -y -f lavfi -i testsrc2=r=25:s=720x576 -f lavfi -i sine -pix_fmt yuv420p -c:v dvvideo -aspect 4/3  -t 0.2 -ac 2 -ar 48000 -vf "drawtext=fontfile='andale-mono.ttf':x=(w-text_w)/2:y=(h-text_h)/2:fontsize=36:fontcolor=white:box=1:boxcolor=gray@0.6:boxborderw=12:shadowx=2:shadowy=2:text='DV25 PAL  4\:2\:0 4\3  2Ch 48kHz'" dv25_pal__420_4-3_2ch_48k_bars_sine.dv>/dev/null 2>&1 || fatal "internal" "ffmpeg command failed"
    ffmpeg -y -f lavfi -i testsrc2=r=30000/1001:s=720x480 -f lavfi -i sine -pix_fmt yuv422p -c:v dvvideo -aspect 4/3  -t 0.2 -ac 2 -ar 48000 -vf "drawtext=fontfile='andale-mono.ttf':x=(w-text_w)/2:y=(h-text_h)/2:fontsize=36:fontcolor=white:box=1:boxcolor=gray@0.6:boxborderw=12:shadowx=2:shadowy=2:text='DV50 NTSC 4\:2\:2 4\3  2Ch 48kHz'" dv50_ntsc_422_4-3_2ch_48k_bars_sine.dv >/dev/null 2>&1 || fatal "internal" "ffmpeg command failed"
    ffmpeg -y -f lavfi -i testsrc2=r=25:s=720x576 -f lavfi -i sine -pix_fmt yuv422p -c:v dvvideo -aspect 4/3  -t 0.2 -ac 2 -ar 48000 -vf "drawtext=fontfile='andale-mono.ttf':x=(w-text_w)/2:y=(h-text_h)/2:fontsize=36:fontcolor=white:box=1:boxcolor=gray@0.6:boxborderw=12:shadowx=2:shadowy=2:text='DV50 PAL  4\:2\:2 4\3  2Ch 48kHz'" dv50_pal__422_4-3_2ch_48k_bars_sine.dv >/dev/null 2>&1 || fatal "internal" "ffmpeg command failed"
    cat dv*k_bars_sine.dv > mix.dv

    run_dvrescue mix.dv
    check_success
    check_xml
    result=$(echo "${cmd_stdout}" | xmllint --xpath "count(/*[local-name()='dvrescue']/*[local-name()='media'][last()]/*[local-name()='frames'])" -)

    if [ "${result}" -ne "7" ] ; then
        error "$test" "invalid number of nodes in output xml"
    fi
popd >/dev/null 2>&1
rm -fr "${test}"

exit ${status}
