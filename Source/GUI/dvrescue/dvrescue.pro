TEMPLATE = subdirs

QTAV = $$(QTAV)
isEmpty(QTAV) {
    QTAV=$$absolute_path(dvrescue-qtav)
} else {
    QTAV=$$absolute_path($$QTAV)
}

message('QTAV: ' $$QTAV)

oldConf = $$cat($$QTAV/.qmake.conf.backup, lines)
isEmpty(oldConf) {
    oldConf = $$cat($$QTAV/.qmake.conf, lines)
    message('writting backup of original .qmake.conf')
    write_file($$QTAV/.qmake.conf.backup, oldConf)
} else {
    message('reading backup of original .qmake.conf.backup')
}

message('oldConf: ' $$oldConf)
write_file($$QTAV/.qmake.conf, oldConf)

USE_BREW = $$(USE_BREW)
!isEmpty(USE_BREW):equals(USE_BREW, true) {
    message("DEFINES += USE_BREW")
    DEFINES += USE_BREW
}

include(ffmpeg.pri)

contains(DEFINES, USE_BREW) {
    message('using ffmpeg from brew via PKGCONFIG')

    pkgConfig = "PKGCONFIG += libavdevice libavcodec libavfilter libavformat libpostproc libswresample libswscale libavcodec libavutil"
    linkPkgConfig = "CONFIG += link_pkgconfig"

    message('pkgConfig: ' $$pkgConfig)
    message('linkPkgConfig: ' $$linkPkgConfig)

    write_file($$QTAV/.qmake.conf, pkgConfig, append)
    write_file($$QTAV/.qmake.conf, linkPkgConfig, append)
} else {
    ffmpegIncludes = "INCLUDEPATH+=$$FFMPEG_INCLUDES"
    ffmpegLibs = "LIBS+=$$FFMPEG_LIBS"

    message('ffmpegIncludes: ' $$ffmpegIncludes)
    message('ffmpegLibs: ' $$ffmpegLibs)

    staticffmpeg = "CONFIG += static_ffmpeg"
    message('staticffmpeg: ' $$ffmpegLibs)

    write_file($$QTAV/.qmake.conf, ffmpegIncludes, append)
    write_file($$QTAV/.qmake.conf, ffmpegLibs, append)
    write_file($$QTAV/.qmake.conf, staticffmpeg, append)

    # to fix building QtAV with the latest ffmpeg
    limitMacros = "DEFINES += __STDC_LIMIT_MACROS"
    write_file($$QTAV/.qmake.conf, limitMacros, append)
}

linux: {
    fpic = "QMAKE_CXXFLAGS += -fPIC"
    write_file($$QTAV/.qmake.conf, fpic, append)
}

mac: {
    noVideoToolbox = "CONFIG*=no-videotoolbox"
    write_file($$QTAV/.qmake.conf, noVideoToolbox, append)
}

# .. we have to build examples, otherwise QML-related dependencies not copied into bin folder
# noExamples = CONFIG*=no-examples
# write_file($$QTAV/.qmake.conf, noExamples, append)

# update: unfortuantely building tests causes some linking errors on Ubuntu 18.04
# so disable it for now
noTests = CONFIG*=no-tests
write_file($$QTAV/.qmake.conf, noTests, append)

SUBDIRS += \
        dvrescue-qtav \
	dvrescue \
        dvrescue_tests \
        dvrescue_qmltests

dvrescue-qtav.file = dvrescue-qtav/QtAV.pro

dvrescue.subdir = dvrescue
dvrescue_tests.subdir = dvrescue.tests
dvrescue_qmltests.subdir = dvrescue.qmltests

dvrescue.depends = dvrescue-qtav
dvrescue_tests.depends = dvrescue-qtav
dvrescue_qmltests.depends = dvrescue-qtav
