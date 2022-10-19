import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import Qt.labs.settings 1.0
import Launcher 0.1
import FileUtils 1.0
import SettingsUtils 1.0
import QwtQuick2 1.0

Dialog {
    id: aboutDialog
    property string version: ""
    property string buildDate: ""
    property string buildQtVersion: ""
    property string runtimeQtVersion: ""
    property string buildFFmpegVersion: ""
    property string runtimeFFmpegVersion: ""

    anchors.centerIn: parent
    contentWidth: layout.childrenRect.width
    contentHeight: layout.childrenRect.height
    standardButtons: Dialog.Close

    ColumnLayout {
        id: layout
        Image {
            source: "/dvrescue.png"
            Layout.alignment: Qt.AlignHCenter
        }

        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            Text {
                text: "Version"
                font.pixelSize: 20
            }

            Text {
                text: version
                font.pixelSize: 20
            }

            Text {
                text: ' - '
            }

            Text {
                text: buildDate
                font.pixelSize: 20
            }
        }

        Text {
            text: "<a href='https://www.mipops.org/'>Copyright © 2019-2022, Moving Image Preservation of Puget Sound.</a>"
            Layout.alignment: Qt.AlignHCenter
            onLinkActivated: {
                console.debug('link: ', link)
                Qt.openUrlExternally(link)
            }
            MouseArea {
                anchors.fill: parent
                acceptedButtons: Qt.NoButton
                cursorShape: Qt.PointingHandCursor
            }
        }

        Text {
            text: "<a href='https://mediaarea.net/'>Third party libraries Copyright © 2012-2020 MediaArea.net SARL</a>"
            Layout.alignment: Qt.AlignHCenter
            onLinkActivated: {
                console.debug('link: ', link)
                Qt.openUrlExternally(link)
            }
            MouseArea {
                anchors.fill: parent
                acceptedButtons: Qt.NoButton
                cursorShape: Qt.PointingHandCursor
            }
        }

        Text {
            text: "Build Qt version: " + buildQtVersion + ", runtime Qt version: " + runtimeQtVersion
            Layout.alignment: Qt.AlignHCenter
        }

        Text {
            text: "This software uses libraries from the FFmpeg project under the LGPLv2.1"
            Layout.alignment: Qt.AlignHCenter
        }

        Text {
            text: "Build FFmpeg version: " + buildFFmpegVersion + ", runtime FFmpeg version: " + runtimeFFmpegVersion
            Layout.alignment: Qt.AlignHCenter
        }
    }
}
