TEMPLATE = app

include(../dvrescue/dvrescue.pri)
include(../ffmpeg.pri)

CONFIG += c++17 warn_on
CONFIG += console

QT += quick testlib widgets multimedia multimediawidgets
equals(QT_MAJOR_VERSION, 5): QT += xmlpatterns

SOURCES += \
    csvparsingtest.cpp \
    datamodeltest.cpp \
    dummytest.cpp \
    main.cpp \
    playertest.cpp \
    xmlparsingtest.cpp

QML_ROOT_PATH = $$absolute_path($$PWD/../dvrescue)
message('QML_ROOT_PATH: ' $$QML_ROOT_PATH)

DISTFILES += \
    tests/tst_deviceslist.qml \
    tests/tst_parsing.qml

HEADERS += \
    csvparsingtest.h \
    datamodeltest.h \
    dummytest.h \
    playertest.h \
    xmlparsingtest.h

RESOURCES += \
    testdata.qrc

DEFINES += QT_AVPLAYER_MULTIMEDIA
INCLUDEPATH += ../dvrescue-QtAVPlayer/src
include(../dvrescue-QtAVPlayer/src/QtAVPlayer/QtAVPlayer.pri)