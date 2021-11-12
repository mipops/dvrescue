TEMPLATE = app

include(../dvrescue/dvrescue.pri)
include(../ffmpeg.pri)

CONFIG += c++17 warn_on
CONFIG += no_keywords
CONFIG += console

QT += quick testlib xmlpatterns widgets multimedia multimediawidgets

SOURCES += \
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
    datamodeltest.h \
    dummytest.h \
    playertest.h \
    xmlparsingtest.h

RESOURCES += \
    testdata.qrc

QTAVPLAYER_SRC=$$absolute_path(../dvrescue-QtAVPlayer)
QTAVPLAYER_LIB=$$absolute_path($$OUT_PWD/../dvrescue-QtAVPlayer)
include(../dvrescue-QtAVPlayer/UseQtAVPlayerLib.pri)
