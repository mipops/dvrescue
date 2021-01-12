import QtQuick 2.12
import QtQuick.Layouts 1.11
import QtQuick.Controls 2.15

Rectangle {
    id: fileViewer
    height: 100
    color: "#ffffff"
    Rectangle {
        id: rectangle9
        x: 0
        y: 0
        anchors.left: parent.left
        anchors.right: parent.right
        height: 50
        color: "#dbdbdb"
        border.color: "#bebdbd"

        Button {
            anchors.right: parent.right
            anchors.top: parent.top
            text: qsTr("Add files")
        }

        Text {
            anchors.left: parent.left
            anchors.top: parent.top
            text: qsTr("File Viewer")
            font.pixelSize: 12
        }
    }
}
