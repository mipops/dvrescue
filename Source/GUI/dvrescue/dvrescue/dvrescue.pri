include(./qwt.pri)

lessThan(QT_MAJOR_VERSION, 6):lessThan(QT_MINOR_VERSION, 13) RESOURCES += $$PWD/splitview_qt5.12.qrc

lessThan(QT_MAJOR_VERSION, 6): {
    RESOURCES += $$PWD/graphicaleffects_qt5.qrc
    RESOURCES += $$PWD/multimedia_qt5.qrc
    RESOURCES += $$PWD/dialogs_qt5.qrc
    QML_IMPORT_PATH += $$PWD/qt5

    lessThan(QT_MINOR_VERSION, 13): {
        QML_IMPORT_PATH += $$PWD/qt512
        RESOURCES += $$PWD/splitview_qt5.12.qrc
    }
}
equals(QT_MAJOR_VERSION, 6): {
    RESOURCES += $$PWD/graphicaleffects.qrc
    RESOURCES += $$PWD/multimedia.qrc
    RESOURCES += $$PWD/dialogs.qrc
    QML_IMPORT_PATH += $$PWD/qt6
}

versionAtLeast(QT_VERSION, 5.13.0): {
    QML_IMPORT_PATH += $$PWD/qt513+
    RESOURCES += $$PWD/splitview_qt5.13+.qrc
}

RESOURCES += $$PWD/qml.qrc

HEADERS += \
    $$PWD/clipboard.h \
    $$PWD/connectionutils.h \
    $$PWD/datamodel.h \
    $$PWD/filewriter.h \
    $$PWD/imageutils.h \
    $$PWD/launcher.h \
    $$PWD/fileutils.h \
    $$PWD/logging.h \
    $$PWD/loggingutils.h \
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

macx:HEADERS += $$PWD/machelpers.h

SOURCES += \
    $$PWD/clipboard.cpp \
    $$PWD/connectionutils.cpp \
    $$PWD/datamodel.cpp \
    $$PWD/filewriter.cpp \
    $$PWD/imageutils.cpp \
    $$PWD/launcher.cpp \
    $$PWD/fileutils.cpp \
    $$PWD/logging.cpp \
    $$PWD/loggingutils.cpp \
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

macx:SOURCES += $$PWD/machelpers.mm

INCLUDEPATH += $$PWD
INCLUDEPATH += $$PWD/3rdparty

DISTFILES += \
    $$PWD/fonts/DS-DIGI.TTF

message("QML_IMPORT_PATH: " $$QML_IMPORT_PATH)
