import QtQuick 2.0
import QtQuick.Controls 2.12
import QtQuick 2.12

Rectangle {
    id: textDelegate
    implicitWidth: 100
    implicitHeight: 20
    color: (row % 2) == 0 ? 'gray' : 'lightgray'
    property alias overlayColor: overlay.color
    property alias overlayVisible: overlay.visible
    property alias text: textLabel.text
    property alias textFont: textLabel.font
    property alias evenProgress: evenProgress
    property alias oddProgress: oddProgress
    property color evenProgressColor: 'transparent'
    property color oddProgressColor: 'transparent'

    ProgressBar {
        id: evenProgress
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.rightMargin: parent.width / 2
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        rotation: 180

        background: Rectangle {
            color: textDelegate.color
        }

        contentItem: Item {
            Rectangle {
                width: evenProgress.visualPosition * parent.width
                height: parent.height
                color: evenProgressColor
            }
        }
    }

    ProgressBar {
        id: oddProgress

        anchors.left: parent.left
        anchors.leftMargin: parent.width / 2
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom

        background: Rectangle {
            color: textDelegate.color
        }

        contentItem: Item {
            Rectangle {
                width: oddProgress.visualPosition * parent.width
                height: parent.height
                color: oddProgressColor
            }
        }
    }

    Rectangle {
        anchors.fill: textLabel
        color: textDelegate.color
        opacity: 0.5
    }

    TextInput {
        id: textLabel
        text: display
        anchors.centerIn: parent
        readOnly: true
    }

    Rectangle {
        id: overlay
        anchors.fill: parent
        opacity: 0.5
        visible: false
    }
}
