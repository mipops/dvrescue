#!/usr/bin/env bash

# Test shell tools: verify they exist, are executable, and respond to help flags

script_path="${PWD}/test"
. ${script_path}/helpers.sh

tools_path="${PWD}/../../../tools"

status=0

# Helper: check that a tool script exists and is executable
check_tool_exists() {
    local tool="${1}"
    local path="${tools_path}/${tool}"
    if [[ ! -f "${path}" ]] ; then
        error "tools/${tool}" "tool not found at ${path}"
        return 1
    fi
    if [[ ! -x "${path}" ]] ; then
        error "tools/${tool}" "tool is not executable"
        return 1
    fi
    echo "OK: tools/${tool} exists and is executable" >&${fd}
    return 0
}

# Helper: check that a tool responds to -h with usage info
check_tool_help() {
    local tool="${1}"
    local path="${tools_path}/${tool}"

    # Run with -h and capture stderr+stdout (most tools print to stderr)
    local output
    output="$("${path}" -h 2>&1)"
    local ret=$?

    # Some tools exit 0 on help, some exit 1 — just check output contains usage info
    if echo "${output}" | grep -qi "usage\|options\|help" ; then
        echo "OK: tools/${tool} -h produces help output" >&${fd}
        return 0
    else
        error "tools/${tool}" "-h does not produce help output"
        return 1
    fi
}

# Test each shell tool
for tool in dvplay dvpackager dvloupe dvsampler ; do
    check_tool_exists "${tool}"
    if [[ $? -eq 0 ]] ; then
        check_tool_help "${tool}"
    fi
done

# Test dvplay --loupe dispatches to dvloupe
if [[ -x "${tools_path}/dvplay" ]] && [[ -x "${tools_path}/dvloupe" ]] ; then
    output="$("${tools_path}/dvplay" --loupe -h 2>&1)"
    if echo "${output}" | grep -qi "dvloupe\|DIF Block\|hex" ; then
        echo "OK: dvplay --loupe dispatches to dvloupe" >&${fd}
    else
        error "tools/dvplay-loupe" "dvplay --loupe -h does not show dvloupe help"
    fi
fi

# Test dvrescue2csv existence (it's a simpler script)
if [[ -f "${tools_path}/dvrescue2csv" ]] ; then
    echo "OK: tools/dvrescue2csv exists" >&${fd}
fi

exit ${status}
