import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import Qt.labs.settings 1.0
import Launcher 0.1
import FileUtils 1.0
import GraphModel 1.0
import QwtQuick2 1.0

QwtQuick2PlotPicker {
    id: picker

    property var canvasItem: picker.parent.canvasItem
    onVisibleChanged: {
        console.debug('visible: ', visible);
    }

    /* debug canvas item geometry
    Rectangle {
        color: 'red'
        parent: videoPlot.canvasItem
        anchors.fill: parent
    }
    */

    Text {
        parent: canvasItem
        x: picker.x + height * 1.5
        y: picker.y - height * 1.5
        text: picker.point.x + ", " + picker.point.y
        visible: picker.active
    }

    Rectangle {
        color: 'green'
        parent: canvasItem
        width: canvasItem.width
        height: 1
        y: picker.y
        visible: picker.active
    }
    Rectangle {
        color: 'green'
        parent: canvasItem
        height: canvasItem.height
        width: 1
        x: picker.x
        visible: picker.active
    }
}
