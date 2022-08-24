import QtQuick 2.0
import QtQuick.Controls 2.12
import QtQuick 2.12

Rectangle {
    id: textDelegate
    implicitWidth: 100
    implicitHeight: 20
    color: (row % 2) == 0 ? 'gray' : 'lightgray'
    property real overlayColorOpacity: 0.5
    property alias overlayColor: overlay.color
    property alias overlayVisible: overlay.visible
    property alias text: textLabel.text
    property alias textFont: textLabel.font
    property int jumpValue: 0
    property alias hasRepeat: repeatImage.visible

    TextInput {
        id: textLabel
        text: display
        readOnly: true
        anchors.centerIn: parent
    }

    Row {
        anchors.left: textLabel.right

        Image {
            source: "/icons/alert-non-continuous-jump-back.svg"
            height: textDelegate.height
            width: height
            visible: jumpValue === 2
        }

        Image {
            source: "/icons/alert-non-continuous-jump-ahead.svg"
            height: textDelegate.height
            width: height
            visible: jumpValue === 1
        }

        Image {
            source: "/icons/alert-non-continuous-jump.svg"
            height: textDelegate.height
            width: height
            visible: jumpValue > 2 || jumpValue < 0
        }

        Image {
            id: repeatImage
            source: "/icons/alert-repeat-frame.svg"
            height: textDelegate.height
            width: height
            visible: false
        }
    }

    Rectangle {
        id: overlay
        anchors.fill: parent
        opacity: overlayColorOpacity
        visible: false
    }
}
