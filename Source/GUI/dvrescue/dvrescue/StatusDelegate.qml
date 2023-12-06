import QtQuick 2.0
import QtQuick 2.12
import TableModel 1.0
import TableModelColumn 1.0
import SortFilterTableModel 1.0
import QtQuick.Controls 2.12
import QtQml 2.12
import Qt.labs.qmlmodels 1.0
import MediaInfo 1.0
import FileUtils 1.0

Rectangle {
    id: statusDelegate
    implicitWidth: 100
    implicitHeight: 20
    color: (row % 2) == 0 ? 'gray' : 'lightgray'
    property real overlayColorOpacity: 0.5
    property alias overlayColor: overlay.color
    property alias overlayVisible: overlay.visible
    property string status: ''

    Row {
        anchors.centerIn: parent

        Repeater {
            id: statusVisualizer
            model: status.length

            delegate: Rectangle {
                width: 20
                height: 20
                radius: 10
                color: status[index] === ' ' ? 'green' : status[index] === 'P' ? 'red' : 'gray'
                border.width: 1
                border.color: 'white'
            }
        }
    }

    Rectangle {
        id: overlay
        anchors.fill: parent
        opacity: overlayColorOpacity
        visible: false
    }
}
