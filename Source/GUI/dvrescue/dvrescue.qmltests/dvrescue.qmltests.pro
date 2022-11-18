TEMPLATE = app

include(../dvrescue/dvrescue.pri)
include(../ffmpeg.pri)

CONFIG += c++17 warn_on qmltestcase
CONFIG += no_keywords

QT += quick widgets quickcontrols2 multimedia

SOURCES += \
    main.cpp \
    setup.cpp

QML_ROOT_PATH = $$absolute_path($$PWD/../dvrescue)
message('QML_ROOT_PATH: ' $$QML_ROOT_PATH)

DISTFILES += \
    tests/tst_about.qml \
    tests/tst_captureframeinfo.qml \
    tests/tst_captureview.qml \
    tests/tst_dvloupeview.qml \
    tests/tst_fileview.qml \
    tests/tst_navbar.qml \
    tests/tst_popup.qml \
    tests/tst_segment_table.qml

HEADERS += \
    setup.h

RESOURCES += \
    testdata.qrc

QTAVPLAYER_SRC=$$absolute_path(../dvrescue-QtAVPlayer)
QTAVPLAYER_LIB=$$absolute_path($$OUT_PWD/../dvrescue-QtAVPlayer)
include(../dvrescue-QtAVPlayer/UseQtAVPlayerLib.pri)
