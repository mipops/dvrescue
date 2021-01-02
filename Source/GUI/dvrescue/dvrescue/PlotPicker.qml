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
        console.debug('QwtQuick2PlotPicker visible: ', visible);
    }

    /* debug canvas item geometry
    Rectangle {
        color: 'red'
        parent: videoPlot.canvasItem
        anchors.fill: parent
    }
    */

    Text {
        id: overlayText
        parent: canvasItem
        x: picker.x + height
        y: picker.y - height * 1.5
        text: Math.abs(picker.point.x).toFixed(0) + ", " + Math.abs(picker.point.y).toFixed(0)
        visible: picker.visible && mouseArea.containsMouse
    }

    Rectangle {
        color: 'green'
        parent: canvasItem
        width: canvasItem.width
        height: 1
        y: picker.y
        visible: picker.visible && picker.active
    }
    Rectangle {
        color: 'green'
        parent: canvasItem
        height: canvasItem.height
        width: 1
        x: picker.x
        visible: picker.visible && picker.active
    }

    MouseArea {
        id: mouseArea
        parent: canvasItem
        anchors.fill: parent
        cursorShape: picker.visible ? Qt.CrossCursor : Qt.ArrowCursor
        hoverEnabled: true

        onMouseXChanged: {
            picker.point = picker.invTransform(Qt.point(mouse.x, mouse.y))
            picker.x = mouse.x
            mouse.accepted = false;
        }
        onMouseYChanged: {
            picker.point = picker.invTransform(Qt.point(mouse.x, mouse.y))
            picker.y = mouse.y
            mouse.accepted = false;
        }
        onPressed: {
            mouse.accepted = false;
        }
        onReleased: {
            mouse.accepted = false;
        }
    }
}
