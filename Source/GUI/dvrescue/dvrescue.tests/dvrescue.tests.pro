TEMPLATE = app

include(../dvrescue/dvrescue.pri)

CONFIG += c++17 warn_on
CONFIG += no_keywords

QT += quick testlib xmlpatterns widgets

SOURCES += \
    dummytest.cpp \
    main.cpp \
    xmlparsingtest.cpp

QML_ROOT_PATH = $$absolute_path($$PWD/../dvrescue)
message('QML_ROOT_PATH: ' $$QML_ROOT_PATH)

DISTFILES += \
    tests/tst_deviceslist.qml \
    tests/tst_parsing.qml

HEADERS += \
    dummytest.h \
    xmlparsingtest.h
