TEMPLATE = app

CONFIG += warn_on qmltestcase
QT += quick

SOURCES += \
    main.cpp

RESOURCES += $$PWD/../dvrescue/qml.qrc

QML_ROOT_PATH = $$absolute_path($$PWD/../dvrescue)
message('QML_ROOT_PATH: ' $$QML_ROOT_PATH)

DISTFILES += \
    tests/tst_deviceslist.qml \
    tests/tst_parsing.qml
