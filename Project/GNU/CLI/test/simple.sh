#!/usr/bin/env bash

script_path="${PWD}/test"
. ${script_path}/helpers.sh

# check version
run_dvrescue --version
check_success
if [ "${?}" -ne 0 ] ; then
    error "simple/version" "command failed"
fi

if [[ ! "${cmd_stdout}" =~ ^DVRescue\ v\.([0-9A-Za-z]+\.)+[0-9A-Za-z]+\ \(MediaInfoLib\ v\.([0-9A-Za-z]+\.)+[0-9A-Za-z]+\)\ by\ MIPoPS\.$ ]] ; then
    error "simple/version" "invalid version string"
fi

# check usage
run_dvrescue
check_failure
if [ "${?}" -ne 0 ] ; then
    error "simple/usage" "invalid return code"
fi

if ! contains "Usage: " "${cmd_stdout}" ; then
    error "simple/usage" "invalid usage string"
fi

# check help
run_dvrescue --help
check_success
if [ "${?}" -ne 0 ] ; then
    error "simple/help" "command failed"
fi

if ! contains "Options:" "${cmd_stdout}" ; then
    error "simple/help" "invalid help string ${cmd_stdout}"
fi

exit ${status}
