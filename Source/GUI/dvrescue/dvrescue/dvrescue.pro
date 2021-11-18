QT += qml quick widgets quickcontrols2 multimedia

CONFIG += c++17
CONFIG += no_keywords

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

macx {
    ICON = icons/icon.icns
    QMAKE_INFO_PLIST = dvrescue.plist
}

win32 {
    RC_ICONS = icons/icon.ico
}

SOURCES += main.cpp

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

QTAVPLAYER_SRC=$$absolute_path(../dvrescue-QtAVPlayer)
QTAVPLAYER_LIB=$$absolute_path($$OUT_PWD/../dvrescue-QtAVPlayer)

message('using UseQtAVPlayerLib')
include(../dvrescue-QtAVPlayer/UseQtAVPlayerLib.pri)

include(./dvrescue.pri)
include(../ffmpeg.pri)

message('QMAKE_POST_LINK: ' $${QMAKE_POST_LINK})
