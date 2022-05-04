TEMPLATE = subdirs

USE_BREW = $$(USE_BREW)
!isEmpty(USE_BREW):equals(USE_BREW, true) {
    message("DEFINES += USE_BREW")
    DEFINES += USE_BREW
}

include(ffmpeg.pri)

QTAVPLAYER = $$absolute_path($$_PRO_FILE_PWD_/dvrescue-QtAVPlayer)
message('QTAVPLAYER: ' $$QTAVPLAYER)

QTAVPLAYERLIB = $$absolute_path($$_PRO_FILE_PWD_/dvrescue-QtAVPlayer-lib)
message('QTAVPLAYER library support files: ' $$QTAVPLAYERLIB)

defineReplace(nativePath) {
    OUT_NATIVE_PATH = $$1
    # Replace slashes in paths with backslashes for Windows
    win32:OUT_NATIVE_PATH ~= s,/,\\,g
    return($$OUT_NATIVE_PATH)
}

win32: {
    system('xcopy $$nativePath($$QTAVPLAYERLIB/*) $$nativePath($$QTAVPLAYER/) /E /Y')
    message('xcopy $$nativePath($$QTAVPLAYERLIB/*) $$nativePath($$QTAVPLAYER/) /E /Y')
} else {
    macx: {
        system('cp -R $$shell_path($$QTAVPLAYERLIB/*) $$shell_path($$QTAVPLAYER/)')
        message('cp -R $$shell_path($$QTAVPLAYERLIB/*) $$shell_path($$QTAVPLAYER/)')
    } else {
        system('cp -r -u $$shell_path($$QTAVPLAYERLIB/*) $$shell_path($$QTAVPLAYER/)')
        message('cp -r -u $$shell_path($$QTAVPLAYERLIB/*) $$shell_path($$QTAVPLAYER/)')
    }
}

contains(DEFINES, USE_BREW) {
    message('using ffmpeg from brew via PKGCONFIG')

    oldConf = $$cat($$QTAVPLAYER/.qmake.conf.backup, lines)
    isEmpty(oldConf) {
        oldConf = $$cat($$QTAVPLAYER/.qmake.conf, lines)
        message('writting backup of original .qmake.conf')
        write_file($$QTAVPLAYER/.qmake.conf.backup, oldConf)
    } else {
        message('reading backup of original .qmake.conf.backup')
    }

    message('oldConf: ' $$oldConf)
    write_file($$QTAVPLAYER/.qmake.conf, oldConf)

    pkgConfig = "PKGCONFIG += libavdevice libavcodec libavfilter libavformat libpostproc libswresample libswscale libavcodec libavutil"
    linkPkgConfig = "CONFIG += link_pkgconfig"

    message('pkgConfig: ' $$pkgConfig)
    message('linkPkgConfig: ' $$linkPkgConfig)

    write_file($$QTAVPLAYER/.qmake.conf, pkgConfig, append)
    write_file($$QTAVPLAYER/.qmake.conf, linkPkgConfig, append)
} else {
    message('including $$QTAVPLAYER/QtAVPlayerLib.pri')
    include($$QTAVPLAYER/QtAVPlayerLib.pri)
}

SUBDIRS += \
        dvrescue-qtavplayer \
	dvrescue \
        #dvrescue_tests \
        #dvrescue_qmltests

dvrescue-qtavplayer.file = dvrescue-QtAVPlayer/QtAVPlayerLib.pro

dvrescue.subdir = dvrescue
#dvrescue_tests.subdir = dvrescue.tests
#dvrescue_qmltests.subdir = dvrescue.qmltests

dvrescue.depends = dvrescue-qtavplayer
#dvrescue_tests.depends = dvrescue-qtavplayer
#dvrescue_qmltests.depends = dvrescue-qtavplayer
