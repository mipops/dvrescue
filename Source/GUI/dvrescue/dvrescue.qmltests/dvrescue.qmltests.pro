TEMPLATE = app

include(../dvrescue/dvrescue.pri)
include(../dvrescue/qtav.pri)
include(../ffmpeg.pri)

CONFIG += c++17 warn_on qmltestcase
CONFIG += no_keywords

QT += quick widgets quickcontrols2

SOURCES += \
    main.cpp \
    setup.cpp

QML_ROOT_PATH = $$absolute_path($$PWD/../dvrescue)
message('QML_ROOT_PATH: ' $$QML_ROOT_PATH)

DISTFILES += \
    tests/tst_deviceslist.qml \
    tests/tst_fileview.qml \
    tests/tst_navbar.qml \
    tests/tst_parsing.qml \
    tests/tst_popup.qml

HEADERS += \
    setup.h

RESOURCES += \
    testdata.qrc
