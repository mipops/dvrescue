TEMPLATE = app

include(../dvrescue/dvrescue.pri)
include(../ffmpeg.pri)

CONFIG += c++17 warn_on qmltestcase

QT += quick widgets quickcontrols2 multimedia

SOURCES += \
    main.cpp \
    setup.cpp

QML_ROOT_PATH = $$absolute_path($$PWD/../dvrescue)
message('QML_ROOT_PATH: ' $$QML_ROOT_PATH)

DISTFILES += \
    tests/tst_about.qml \
    tests/tst_settings.qml \
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

DEFINES += QT_AVPLAYER_MULTIMEDIA
INCLUDEPATH += ../dvrescue-QtAVPlayer/src
include(../dvrescue-QtAVPlayer/src/QtAVPlayer/QtAVPlayer.pri)