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
    property alias imageSource: image.source
    property alias imageVisible: image.visible

    Image {
        id: image
        width: parent.width
        height: parent.height
        anchors.centerIn: parent
    }

    Rectangle {
        id: overlay
        anchors.fill: parent
        opacity: 0.5
        visible: false
    }
}
