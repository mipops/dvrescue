TEMPLATE = subdirs

USE_BREW = $$(USE_BREW)
!isEmpty(USE_BREW):equals(USE_BREW, true) {
    message("DEFINES += USE_BREW")
    DEFINES += USE_BREW
}

include(ffmpeg.pri)

defineReplace(nativePath) {
    OUT_NATIVE_PATH = $$1
    # Replace slashes in paths with backslashes for Windows
    win32:OUT_NATIVE_PATH ~= s,/,\\,g
    return($$OUT_NATIVE_PATH)
}

contains(DEFINES, USE_BREW) {
    message('using ffmpeg from brew via PKGCONFIG')

    pkgConfig = "PKGCONFIG += libavdevice libavcodec libavfilter libavformat libpostproc libswresample libswscale libavcodec libavutil"
    linkPkgConfig = "CONFIG += link_pkgconfig"

    message('pkgConfig: ' $$pkgConfig)
    message('linkPkgConfig: ' $$linkPkgConfig)
}

SUBDIRS += \
	dvrescue \
        dvrescue_tests \
        dvrescue_qmltests

dvrescue.subdir = dvrescue
dvrescue_tests.subdir = dvrescue.tests
dvrescue_qmltests.subdir = dvrescue.qmltests
