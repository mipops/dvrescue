import QtQuick 2.12
import QtQuick.Layouts 1.11
import QtQuick.Controls 2.15

Rectangle {
    color: "#2e3436"
    property alias text1: text1
    property alias fastForwardMouseArea: fastForwardMouseArea
    property alias playMouseArea: playMouseArea
    property alias stopMouseArea: stopMouseArea
    property alias rewindMouseArea: rewindMouseArea
    property alias settingsMouseArea: settingsMouseArea
    property alias grabMouseArea: grabMouseArea
    property alias deviceNameTextField: deviceNameTextField
    property alias topNav: topNav
    width: 1190

    Rectangle {
        id: rectangle7
        x: 275
        y: 629
        width: 640
        height: 71
        color: "#ffffff"
    }

    Column {
        id: topNav
        x: 8
        y: 11
        width: 1197
        height: 100
        transformOrigin: Item.Center
    }

    Image {
        id: image
        x: 275
        y: 148
        width: 640
        height: 480
        source: "colorbars.jpg"
        autoTransform: true
        sourceSize.height: 640
        sourceSize.width: 480
        fillMode: Image.PreserveAspectFit
    }

    Item {
        x: 190
        y: 11

        Rectangle {
            id: rectangle6
            x: -180
            y: 2
            width: 162
            height: 100
            color: "#ffffff"
        }

        Rectangle {
            id: rectangle
            x: -12
            y: 2
            width: 162
            height: 100
            color: "#a40000"
        }

        Text {
            id: text3
            x: 9
            y: 35
            color: "#ffffff"
            text: qsTr("Transfer")
            font.pixelSize: 26
            font.family: "Tahoma"
            minimumPixelSize: 14
            font.weight: Font.Bold
            minimumPointSize: 14
        }

        Rectangle {
            id: rectangle1
            x: 156
            y: 2
            width: 162
            height: 100
            color: "#ffffff"
        }

        Text {
            id: text4
            x: 176
            y: 35
            color: "#282828"
            text: qsTr("Analysis")
            font.pixelSize: 26
            font.family: "Tahoma"
            minimumPixelSize: 14
            font.weight: Font.Bold
            minimumPointSize: 14
        }

        Rectangle {
            id: rectangle2
            x: 324
            y: 2
            width: 162
            height: 100
            color: "#a40000"
        }

        Text {
            id: text5
            x: 344
            y: 35
            color: "#ffffff"
            text: qsTr("Package")
            font.pixelSize: 26
            font.family: "Tahoma"
            minimumPixelSize: 14
            font.weight: Font.Bold
            minimumPointSize: 14
        }

        Rectangle {
            id: rectangle3
            x: 492
            y: 2
            width: 162
            height: 100
            color: "#ffffff"
        }

        Text {
            id: text6
            x: 507
            y: 35
            color: "#282828"
            text: qsTr("Playback")
            font.pixelSize: 26
            font.family: "Tahoma"
            minimumPixelSize: 14
            font.weight: Font.Bold
            minimumPointSize: 14
        }

        Rectangle {
            id: rectangle4
            x: 660
            y: 2
            width: 162
            height: 100
            color: "#a40000"
        }

        Text {
            id: text7
            x: 680
            y: 35
            color: "#ffffff"
            text: qsTr("Settings")
            font.pixelSize: 26
            font.family: "Tahoma"
            minimumPixelSize: 14
            font.weight: Font.Bold
            minimumPointSize: 14
        }

        MouseArea {
            id: settingsMouseArea
            anchors.fill: text7
        }

        Rectangle {
            id: rectangle5
            x: 828
            y: 2
            width: 162
            height: 100
            color: "#eeeeec"
        }

        Text {
            id: text8
            x: 876
            y: 35
            color: "#282828"
            text: qsTr("Help")
            font.pixelSize: 26
            font.family: "Tahoma"
            minimumPixelSize: 14
            font.weight: Font.Bold
            minimumPointSize: 14
        }
    }
    BorderImage {
        id: borderImage
        x: 23
        y: 16
        width: 132
        height: 92
        source: "dvrescue.png"
        verticalTileMode: BorderImage.Round
        horizontalTileMode: BorderImage.Round
        border.bottom: 3
        border.top: 3
        border.right: 3
        border.left: 3
    }

    Rectangle {
        id: rectangle12
        x: 704
        y: 629
        width: 211
        height: 71
        color: "#ffffff"

        Text {
            id: text1
            x: 40
            y: 17
            text: qsTr("Capture")
            font.pixelSize: 32
            font.weight: Font.Bold
            font.family: "Tahoma"

            MouseArea {
                id: grabMouseArea
                anchors.fill: parent
            }
        }

    }
    TextField {
        id: deviceNameTextField
        x: 275
        y: 119
        width: 237
        height: 38
        readOnly: true
        text: "[0] DV-VCR (Sony GV-D1000)"
        font.family: "Tahoma"
        placeholderText: qsTr("Searching device...")
        font.pointSize: 11
    }

    Image {
        id: image1
        x: 283
        y: 629
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
        id: image2
        x: 547
        y: 629
        width: 71
        height: 71
        source: "icons/fastforward.svg"
        fillMode: Image.PreserveAspectFit

        MouseArea {
            id: fastForwardMouseArea
            anchors.fill: parent
        }
    }

    Image {
        id: image3
        x: 455
        y: 629
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
        x: 373
        y: 629
        width: 71
        height: 71
        source: "icons/stop.svg"
        fillMode: Image.PreserveAspectFit

        MouseArea {
            id: stopMouseArea
            anchors.fill: parent
        }
    }
}

/*##^##
Designer {
    D{i:0;formeditorZoom:0.5}
}
##^##*/

