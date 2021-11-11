import QtQuick 2.12
import QtQuick.Layouts 1.11
import QtQuick.Controls 2.12

Column {
    property alias fastForwardButton: fastForwardButton
    property alias playButton: playButton
    property alias stopButton: stopButton
    property alias rewindButton: rewindButton
    property alias captureButton: captureButton
    property alias deviceNameTextField: deviceNameTextField
    property alias statusText: statusText.text

    Rectangle {
        width: 640
        height: childrenRect.height
        color: "#ffffff"

        TextField {
            id: deviceNameTextField
            width: 640
            readOnly: true
            text: "[0] DV-VCR (Sony GV-D1000)"
            font.family: "Tahoma"
            placeholderText: qsTr("Searching device...")
            font.pointSize: 11
        }
    }

    Image {
        id: image
        width: 640
        height: 480
        source: "colorbars.jpg"
        autoTransform: true
        sourceSize.height: 640
        sourceSize.width: 480
        fillMode: Image.PreserveAspectFit
    }

    Rectangle {
        width: 640
        height: childrenRect.height

        Row {
            id: row
            CaptureViewControlButton {
                id: rewindButton
                icon.source: "icons/rewind.svg"
            }

            CaptureViewControlButton {
                id: playButton
                icon.source: "icons/play.svg"
            }

            CaptureViewControlButton {
                id: stopButton
                icon.source: "icons/stop.svg"
            }

            CaptureViewControlButton {
                id: fastForwardButton
                icon.source: "icons/fastforward.svg"
            }
        }

        Text {
            id: statusText
            anchors.left: row.right
            anchors.right: captureButton.left
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            anchors.top: parent.top
            anchors.bottom: parent.bottom
        }

        CaptureViewControlButton {
            id: captureButton
            anchors.right: parent.right
            icon.source: "icons/capture.svg"
        }
    }
}
