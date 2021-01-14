message('qtav.pri')

win32: {
    QTAVLIBFOLDER=lib_win_x86_64
}
linux: {
    QTAVLIBFOLDER=lib_linux_x86_64
}
mac: {
    QTAVLIBFOLDER=lib_osx_x86_64_llvm
}

QTAV = $$QTAV
QTAV_FOLDER_NAME = QtAV
isEmpty(QTAV) {
    message('$$TARGET: using default location for QTAV: ' $$QTAV)
    QTAV_FOLDER_NAME = dvrescue-qtav
    QTAV=$$absolute_path($$PWD/../$$QTAV_FOLDER_NAME)
}

message('qctools-gui: QTAV: ' $$QTAV ', QTAV_FOLDER_NAME: ' $$QTAV_FOLDER_NAME)

message('QTAVLIBFOLDER: ' $$QTAVLIBFOLDER)
INCLUDEPATH += $$absolute_path($$QTAV/src) $$absolute_path($$QTAV/src/QtAV)

if(equals(MAKEFILE_GENERATOR, MSVC.NET)|equals(MAKEFILE_GENERATOR, MSBUILD)) {
  TRY_COPY = $$QMAKE_COPY
} else {
  TRY_COPY = -$$QMAKE_COPY #makefile. or -\$\(COPY_FILE\)
}

message('TRY_COPY: ' $$TRY_COPY)

QTAV_OUT=$$absolute_path($$OUT_PWD/../$$QTAV_FOLDER_NAME/$$QTAVLIBFOLDER)

message('QTAV_OUT: ' $$QTAV_OUT)

mac: {
    include($$QTAV/common.pri)
    QTAVLIBS = -F$$QTAV_OUT -framework QtAV$$platformTargetSuffix()

    qtavlibs.pattern = $$QTAV_OUT/$${QTAVLIBNAME}*
    message('qtavlibs.pattern: ' $$qtavlibs.pattern)

    qtavlibs.files = $$files($$qtavlibs.pattern)
    message('qtavlibs.files: ' $$qtavlibs.files)

    qtavlibs.path = $$absolute_path($$OUT_PWD$${BUILD_DIR}/$${TARGET}.app/Contents/Frameworks)
    message('qtavlibs.path: ' $$qtavlibs.path)

    qtavlibs.commands += $$escape_expand(\\n\\t)$$QMAKE_MKDIR_CMD $$shell_path($$qtavlibs.path)

    for(f, qtavlibs.files) {
      qtavlibs.commands += $$escape_expand(\\n\\t)$$QMAKE_COPY_DIR $$shell_path($$f) $$shell_path($$qtavlibs.path)
    }

    qtavlibs.srcfolder = $$QTAV_OUT/../bin/QtAV
    qtavlibs.dstfolder = $$absolute_path($$OUT_PWD$${BUILD_DIR}/$${TARGET}.app/Contents/MacOS/QtAV)
    qtavlibs.commands += $$escape_expand(\\n\\t)rm -rf $$shell_path($$qtavlibs.dstfolder)
    qtavlibs.commands += $$escape_expand(\\n\\t)$$QMAKE_COPY_DIR $$shell_path($$qtavlibs.srcfolder) $$shell_path($$qtavlibs.dstfolder)

    isEmpty(QMAKE_POST_LINK): QMAKE_POST_LINK = $$qtavlibs.commands
    else: QMAKE_POST_LINK = $${QMAKE_POST_LINK}$$escape_expand(\\n\\t)$$qtavlibs.commands

} else {
    win32: {
        CONFIG(debug, debug|release) {
            BUILD_SUFFIX=d
            BUILD_DIR=/debug
        } else:CONFIG(release, debug|release) {
            BUILD_SUFFIX=
            BUILD_DIR=/release
        }
        QTAVLIBNAME = QtAV$${BUILD_SUFFIX}1
    } else {
        QTAVLIBNAME = QtAV$${BUILD_SUFFIX}
        QTLIBPREFIX = lib
    }
    message('QTAVLIBNAME: ' $${QTAVLIBNAME})

    QTAVLIBS += -L$$QTAV_OUT -l$${QTAVLIBNAME}
    qtavlibs.pattern = $$QTAV_OUT/$$QTLIBPREFIX$${QTAVLIBNAME}.$$QMAKE_EXTENSION_SHLIB*
    message('qtavlibs.pattern: ' $$qtavlibs.pattern)

    qtavlibs.files = $$files($$qtavlibs.pattern)
    message('qtavlibs.files: ' $$qtavlibs.files)

    qtavlibs.path = $$absolute_path($$OUT_PWD$${BUILD_DIR})
    for(f, qtavlibs.files) {
      qtavlibs.commands += $$escape_expand(\\n\\t)$$TRY_COPY $$shell_path($$f) $$shell_path($$qtavlibs.path)
    }

    qtavlibs.srcfolder = $$QTAV_OUT/../bin/QtAV
    qtavlibs.dstfolder = $$absolute_path($$OUT_PWD$${BUILD_DIR}/QtAV)
    win32: {
        qtavlibs.commands += $$escape_expand(\\n\\t)if exist $$shell_path($$qtavlibs.dstfolder) rd /S /Q $$shell_path($$qtavlibs.dstfolder)
    } else {
        qtavlibs.commands += $$escape_expand(\\n\\t)rm -rf $$shell_path($$qtavlibs.dstfolder)
    }
    qtavlibs.commands += $$escape_expand(\\n\\t)$$QMAKE_COPY_DIR $$shell_path($$qtavlibs.srcfolder) $$shell_path($$qtavlibs.dstfolder)
    qtavlibs.commands += $$escape_expand(\\n\\t)

    message('qtavlibs.commands: ' $$qtavlibs.commands)

    isEmpty(QMAKE_POST_LINK): QMAKE_POST_LINK = $$qtavlibs.commands
    else: QMAKE_POST_LINK = $${QMAKE_POST_LINK}$$escape_expand(\\n\\t)$$qtavlibs.commands
}

message('QTAVLIBS: ' $$QTAVLIBS)
LIBS += $$QTAVLIBS


message('// qtav.pri:' $$QMAKE_POST_LINK)
