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
    property alias imageSouce: image.source
    property alias imageVisible: image.visible

    TextInput {
        id: textLabel
        text: display
        anchors.centerIn: parent
        readOnly: true
    }

    Image {
        id: image
        anchors.verticalCenter: parent.verticalCenter
        anchors.right: textLabel.left
        height: textDelegate.height
        width: height
    }

    Rectangle {
        id: overlay
        anchors.fill: parent
        opacity: 0.5
        visible: false
    }
}
