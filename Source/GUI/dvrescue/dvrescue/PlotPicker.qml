import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import Qt.labs.settings 1.0
import Launcher 0.1
import FileUtils 1.0
import GraphModel 1.0
import QwtQuick2 1.0

QwtQuick2PlotPicker {
    id: picker

    enum ActionType {
        Picker = 0,
        Zooming = 1,
        Moving = 2
    }

    signal zoomed(int x1, int x2);
    signal moved(int x1)

    property alias active: picker.active
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
        text: {
            if(mouseArea.actionType === PlotPicker.ActionType.Picker) {
                return overlayTextFormatter ? overlayTextFormatter(picker.point) :
                                              Math.abs(picker.point.x).toFixed(0) + ", " + Math.abs(picker.point.y).toFixed(0)
            } else if(mouseArea.actionType === PlotPicker.ActionType.Zooming) {
                return "zooming: " + zoomArea.start + " - " + zoomArea.end
            } else if(mouseArea.actionType === PlotPicker.ActionType.Moving) {
                return "moving...";
            }
        }

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
        visible: mouseArea.actionType === PlotPicker.ActionType.Zooming

        readonly property int start: picker.invTransform(Qt.point(x,0)).x;
        readonly property int end: picker.invTransform(Qt.point(x + width, 0)).x;

        function handleMousePress(mouse) {
            x = mouse.x
        }

        function handleMouseMove(mouse, zoomStart) {
            if(mouse.x > zoomStart) {
                x = zoomStart
                width = mouse.x - zoomStart
            } else {
                x = mouse.x
                width = zoomStart - mouse.x
            }
        }
    }

    MouseArea {
        id: mouseArea
        parent: canvasItem
        anchors.fill: parent
        cursorShape: picker.visible ? Qt.CrossCursor : Qt.ArrowCursor
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        hoverEnabled: true
        property int pressX: 0
        property int startX1: 0

        property int actionType: PlotPicker.ActionType.Picker
        onActionTypeChanged: {
            console.debug('actionType changed: ', actionType);
        }

        onMouseXChanged: {
            picker.point = picker.invTransform(Qt.point(mouse.x, mouse.y))
            picker.x = mouse.x
            if(actionType === PlotPicker.ActionType.Zooming) {
                zoomArea.handleMouseMove(mouse, pressX);
            } else if(actionType === PlotPicker.ActionType.Moving) {
                moved(startX1 + picker.invTransform(Qt.point(pressX, 0)).x - picker.invTransform(Qt.point(mouse.x, 0)).x)
            }
        }
        onMouseYChanged: {
            picker.point = picker.invTransform(Qt.point(mouse.x, mouse.y))
            picker.y = mouse.y
        }
        onPressed: {
            console.debug('pressed');
            pressX = mouse.x

            if(mouse.buttons & Qt.RightButton) {
                actionType = PlotPicker.ActionType.Moving
                startX1 = picker.plotItem.xBottomAxisRange.x
            } else if(mouse.modifiers & Qt.ShiftModifier) {
                actionType = PlotPicker.ActionType.Zooming
                zoomArea.handleMousePress(mouse);
            } else {
                mouse.accepted = false
            }
        }
        onCanceled: {
            console.debug('cancelled');
            actionType = PlotPicker.ActionType.Picker;
        }

        onReleased: {
            console.debug('released');
            if(actionType === PlotPicker.ActionType.Zooming) {
                console.debug('zoomArea.start: ', zoomArea.start, 'zoomArea.end: ', zoomArea.end);
                if(zoomArea.start !== zoomArea.end)
                    zoomed(zoomArea.start, zoomArea.end)
            }

            actionType = PlotPicker.ActionType.Picker;
        }
    }
}
