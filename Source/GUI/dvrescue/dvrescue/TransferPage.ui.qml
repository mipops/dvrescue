import QtQuick 2.12
import QtQuick.Layouts 1.11
import QtQuick.Controls 2.12

Rectangle {
    id: rectangle
    color: "#2e3436"
    property alias fastForwardMouseArea: captureView.fastForwardMouseArea
    property alias playMouseArea: captureView.playMouseArea
    property alias stopMouseArea: captureView.stopMouseArea
    property alias rewindMouseArea: captureView.rewindMouseArea
    property alias grabMouseArea: captureView.grabMouseArea
    property alias deviceNameTextField: captureView.deviceNameTextField
    width: 1190
    height: 768

    CaptureView {
        id: captureView
        x: 275
        y: 120
    }

    FileViewer {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.rightMargin: 10
        anchors.topMargin: 10
    }
}

/*##^##
Designer {
    D{i:0;formeditorZoom:0.9}D{i:24}
}
##^##*/

