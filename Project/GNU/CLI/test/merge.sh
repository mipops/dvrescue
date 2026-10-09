#!/usr/bin/env bash

script_path="${PWD}/test"
. ${script_path}/helpers.sh

test="merge"

mkdir "${test}"
pushd "${test}" >/dev/null 2>&1
    # Two identical captures of 4 s of 625/50 (PAL): their merge has every frame and none missing.
    # Timecodes counted at 30 fps made every second from :24 to :00 a gap of 5 frames (issue #1009)
    ffmpeg -y -f lavfi -i testsrc2=r=25:s=720x576 -f lavfi -i sine -pix_fmt yuv420p -c:v dvvideo -t 4 -ac 2 -ar 48000 -f dv pal.dv >/dev/null 2>&1 || fatal "internal" "ffmpeg command failed"
    cp pal.dv pal_copy.dv
    frames=$(( $(wc -c < pal.dv) / 144000 ))

    run_dvrescue pal.dv pal_copy.dv -m merged.dv --merge-log merge.txt
    check_success
    if [ "${?}" -ne 0 ] ; then
        error "${test}/pal" "command failed"
    fi
    if ! cmp -s pal.dv merged.dv ; then
        error "${test}/pal" "merged file is not the same as the inputs"
    fi
    if grep -q "frames are still missing" merge.txt ; then
        error "${test}/pal" "merge log reports missing frames:$(grep "frames are still missing" merge.txt)"
    fi

    run_dvrescue pal.dv pal_copy.dv -m merged_csv.dv --merge-log merge.csv --csv
    check_success
    last=$(tail -n 1 merge.csv | cut -d, -f1)
    if [ "${last}" != "$((frames - 1))" ] ; then
        error "${test}/pal" "last FramePos in the CSV merge log is ${last}, expected $((frames - 1))"
    fi

    # A copy with damaged subcode in frame 50: every timecode pack reads a frame number the rate doesn't have
    # (27 at 25 fps, 35 at any rate). Counting from the frame before can't reach it, so it must be taken as
    # unreadable: the merge has to end, with every frame in place
    for number in 27 35 ; do
        cp pal.dv damaged.dv
        for seq in $(seq 0 11) ; do
            for block in 1 2 ; do
                for pack in $(seq 0 5) ; do
                    offset=$(( 50 * 144000 + seq * 12000 + block * 80 + 3 + pack * 8 + 3 ))
                    if [ "$(od -An -tx1 -j ${offset} -N1 damaged.dv | tr -d ' \n')" = "13" ] ; then
                        printf "\x${number}" | dd of=damaged.dv bs=1 seek=$((offset + 1)) conv=notrunc >/dev/null 2>&1
                    fi
                done
            done
        done
        run_dvrescue pal.dv damaged.dv -m merged_${number}.dv --merge-log merge_${number}.txt
        check_success
        if [ "${?}" -ne 0 ] ; then
            error "${test}/pal_timecode_${number}" "command failed"
        fi
        if ! cmp -s pal.dv merged_${number}.dv ; then
            error "${test}/pal_timecode_${number}" "merged file is not the same as the undamaged input"
        fi
    done
popd >/dev/null 2>&1
rm -fr "${test}"

exit ${status}
