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

    signal zoomed(int x1, int x2);

    property var canvasItem: picker.parent.canvasItem
    onVisibleChanged: {
        console.debug('QwtQuick2PlotPicker visible: ', visible);
    }

    property var overlayTextFormatter: null

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

        property int maxX: canvasItem.width - width
        property int minY: 0

        x: {
            var newX = picker.x + height
            if(newX > maxX)
                newX = maxX

            return newX
        }
        y: {
            var newY = picker.y - height * 1.5
            if(newY < minY)
                newY = minY

            return newY
        }
        text: mouseArea.zooming ? ("zooming: " + zoomArea.start + " - " + zoomArea.end) :
                  (overlayTextFormatter ? overlayTextFormatter(picker.point) :
                                          Math.abs(picker.point.x).toFixed(0) + ", " + Math.abs(picker.point.y).toFixed(0));

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

    Rectangle {
        id: zoomArea
        color: Qt.rgba(0, 1, 0, 0.5)
        parent: canvasItem
        height: canvasItem.height
        visible: mouseArea.zooming

        property int start: picker.invTransform(x);
        property int end: picker.invTransform(x + width);
    }

    MouseArea {
        id: mouseArea
        parent: canvasItem
        anchors.fill: parent
        cursorShape: picker.visible ? Qt.CrossCursor : Qt.ArrowCursor
        hoverEnabled: true
        property bool zooming: false
        property int zoomStart: 0

        onMouseXChanged: {
            picker.point = picker.invTransform(Qt.point(mouse.x, mouse.y))
            picker.x = mouse.x
            if(zooming) {
                if(mouse.x > zoomStart) {
                    zoomArea.x = zoomStart
                    zoomArea.width = mouse.x - zoomStart
                } else {
                    zoomArea.x = mouse.x
                    zoomArea.width = zoomStart - mouse.x
                }
            }
        }
        onMouseYChanged: {
            picker.point = picker.invTransform(Qt.point(mouse.x, mouse.y))
            picker.y = mouse.y
        }
        onPressed: {
            console.debug('pressed');
            if(mouse.modifiers & Qt.ShiftModifier) {
                zoomStart = mouse.x
                zoomArea.x = zoomStart
                zooming = true;
            } else {
                mouse.accepted = false
            }
        }
        onCanceled: {
            console.debug('cancelled');
            if(zooming) {
                zooming = false;
            }
        }

        onReleased: {
            console.debug('released');
            if(zooming) {
                zoomed(zoomArea.start, zoomArea.end)
                zooming = false;
            }
        }
    }
}
