import QtQuick 2.12
import QtQuick.Layouts 1.11
import QtQuick.Controls 2.12

Column {
    property alias fastForwardMouseArea: fastForwardMouseArea
    property alias playMouseArea: playMouseArea
    property alias stopMouseArea: stopMouseArea
    property alias rewindMouseArea: rewindMouseArea
    property alias grabMouseArea: grabMouseArea
    property alias deviceNameTextField: deviceNameTextField

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
            Image {
                id: image1
                width: 71
                height: 71
                source: "icons/rewind.svg"
                fillMode: Image.PreserveAspectFit

                MouseArea {
                    id: rewindMouseArea
                    anchors.fill: parent
                }
            }

            Image {
                id: image3
                width: 71
                height: 71
                source: "icons/play.svg"
                fillMode: Image.PreserveAspectFit

                MouseArea {
                    id: playMouseArea
                    anchors.fill: parent
                }
            }

            Image {
                id: image4
                width: 71
                height: 71
                source: "icons/stop.svg"
                fillMode: Image.PreserveAspectFit

                MouseArea {
                    id: stopMouseArea
                    anchors.fill: parent
                }
            }

            Image {
                id: image2
                width: 71
                height: 71
                source: "icons/fastforward.svg"
                fillMode: Image.PreserveAspectFit

                MouseArea {
                    id: fastForwardMouseArea
                    anchors.fill: parent
                }
            }
        }


        Text {
            id: text1
            text: qsTr("Capture")
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            verticalAlignment: Text.AlignVCenter
            font.pixelSize: 32
            font.weight: Font.Bold
            font.family: "Tahoma"

            MouseArea {
                id: grabMouseArea
                anchors.fill: parent
            }
        }
    }
}
