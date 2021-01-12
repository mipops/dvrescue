import QtQuick 2.12
import QtQuick.Layouts 1.11
import QtQuick.Controls 2.15

Rectangle {
    id: rectangle
    color: "#2e3436"
    property alias settingsMouseArea: settingsMouseArea

    property alias fastForwardMouseArea: captureView.fastForwardMouseArea
    property alias playMouseArea: captureView.playMouseArea
    property alias stopMouseArea: captureView.stopMouseArea
    property alias rewindMouseArea: captureView.rewindMouseArea
    property alias grabMouseArea: captureView.grabMouseArea
    property alias deviceNameTextField: captureView.deviceNameTextField
    property alias topNav: topNav
    width: 1190
    height: 768

    DvRescueLogo {
        id: dvRescueLogo
        anchors.left: parent.left
        anchors.leftMargin: 10
        anchors.top: parent.top
        anchors.topMargin: 10
    }

    Column {
        id: topNav
        anchors.left: parent.left
        anchors.leftMargin: 10
        anchors.top: dvRescueLogo.bottom
        anchors.topMargin: 10
        spacing: 10

        RedNavButton {
            text: qsTr("Transfer")
        }
        WhiteNavButton {
            text: qsTr("Analysis")
        }
        RedNavButton {
            text: qsTr("Package")
        }
        WhiteNavButton {
            text: qsTr("Playback")
        }
        RedNavButton {
            text: qsTr("Settings")
            MouseArea {
                id: settingsMouseArea
                anchors.fill: parent
            }
        }
        WhiteNavButton {
            text: qsTr("Help")
        }
    }

    CaptureView {
        id: captureView
        x: 275
        y: 120
    }

    FileViewer {
        anchors.left: dvRescueLogo.right
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.rightMargin: 10
        anchors.leftMargin: 10
        anchors.topMargin: 10
    }
}

/*##^##
Designer {
    D{i:0;formeditorZoom:0.9}D{i:24}
}
##^##*/

