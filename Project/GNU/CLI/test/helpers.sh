#!/usr/bin/env bash

if type -p md5sum ; then
    md5cmd="md5sum"
elif type -p md5 ; then
    md5cmd="md5 -r"
else
    fatal "internal" "command not found for md5sum" >&${fd}
fi >/dev/null 2>&1

fd=1
if (command exec >&9) ; then
    fd=9
fi >/dev/null 2>&1

PATH="${PWD}:$PATH"

script_path="${PWD}/test"
files_path="${script_path}/TestingFiles"

status=0
timeout=480

fatal() {
    local test="${1}"
    local message="${2}"

    echo "NOK: ${test}, ${message}" >&${fd}
    status=1

    exit ${status}
}

error() {
    local test="${1}"
    local message="${2}"

    echo "NOK: ${test}, ${message}" >&${fd}
    status=1
}

contains() {
    echo "${2}" | grep -q "${1}"
}

run_dvrescue() {
    unset cmd_status
    unset cmd_stdout
    unset cmd_stderr

    local temp="$(mktemp -d -t 'dvrescue_testsuite.XXXXXX')"

    local valgrind=""
    if command -v valgrind && test -n "${VALGRIND}" ; then
        valgrind="valgrind --quiet --track-origins=yes --log-file=${temp}/valgrind"
    fi >/dev/null 2>&1

    ${valgrind} dvrescue $@ >"${temp}/stdout" 2>"${temp}/stderr" & kill -STOP ${!}; local pid=${!}
    sleep ${timeout} && (kill -HUP ${pid} ; fatal "command timeout: dvrescue $@") & local watcher=${!}
    kill -CONT ${pid} ; wait ${pid}
    cmd_status="${?}"
    pkill -P ${watcher}

    cmd_stdout="$(<${temp}/stdout)"
    cmd_stderr="$(<${temp}/stderr)"

    # check valgrind
    if [ -n "${valgrind}" ] && [ -s "${temp}/valgrind" ] ; then
        cat "${temp}/valgrind" >&${fd}
        status=1
    fi

    rm -fr "${temp}"

    return ${cmd_status}
}

check_success() {
    if [ "${?}" -ne 0 ] ; then
        status=1
    fi

    return "${?}"
}

check_failure() {
    if [ "${?}" -eq 0 ] ; then
        status=1
    fi

    return "${?}"
}

check_xml() {
     echo "${cmd_stdout}" | xmllint --path "${script_path}/../../../../tools/" --noout - 2> /dev/null
    check_success

    return "${?}"
}
