include(./qwt.pri)

RESOURCES += $$PWD/qml.qrc

HEADERS += \
    $$PWD/clipboard.h \
    $$PWD/connectionutils.h \
    $$PWD/datamodel.h \
    $$PWD/filewriter.h \
    $$PWD/launcher.h \
    $$PWD/fileutils.h \
    $$PWD/logging.h \
    $$PWD/mediainfo.h \
    $$PWD/mediaplayer.h \
    $$PWD/playbackbuffer.h \
    $$PWD/qqmltablemodel_p.h \
    $$PWD/qqmltablemodelcolumn_p.h \
    $$PWD/qtavplayerutils.h \
    $$PWD/qwtquick2plot.h \
    $$PWD/settingsutils.h \
    $$PWD/thread.h \
    $$PWD/xmlparser.h \
    $$PWD/csvparser.h \
    $$PWD/plotdata.h \
    $$PWD/sortfiltertablemodel.h

SOURCES += \
    $$PWD/clipboard.cpp \
    $$PWD/connectionutils.cpp \
    $$PWD/datamodel.cpp \
    $$PWD/filewriter.cpp \
    $$PWD/launcher.cpp \
    $$PWD/fileutils.cpp \
    $$PWD/logging.cpp \
    $$PWD/mediainfo.cpp \
    $$PWD/mediaplayer.cpp \
    $$PWD/playbackbuffer.cpp \
    $$PWD/qqmltablemodel.cpp \
    $$PWD/qqmltablemodelcolumn.cpp \
    $$PWD/qtavplayerutils.cpp \
    $$PWD/qwtquick2plot.cpp \
    $$PWD/settingsutils.cpp \
    $$PWD/thread.cpp \
    $$PWD/xmlparser.cpp \
    $$PWD/csvparser.cpp \
    $$PWD/plotdata.cpp \
    $$PWD/sortfiltertablemodel.cpp

INCLUDEPATH += $$PWD
INCLUDEPATH += $$PWD/3rdparty

DISTFILES += \
    $$PWD/fonts/DS-DIGI.TTF
