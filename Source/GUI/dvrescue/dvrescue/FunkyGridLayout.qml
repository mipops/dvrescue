import QtQuick 2.11
import QtQuick.Layouts 1.3

Rectangle {
    id: layout

    onChildrenChanged: scheduleLayout()
    onWidthChanged: scheduleLayout()
    onHeightChanged: scheduleLayout()

    Timer {
        id: timer
        running: false
        repeat: false
        interval: 0
        onTriggered: {
            performLayout();
        }
    }

    function scheduleLayout() {
        timer.restart();
    }

    function fitToSpace(item, w, h) {
        if(item.width <= w && item.height <= h) {
            item.scale = 1
        } else {
            if(item.width / item.height > w / h) {
                // item wider than available space? fit by width
                item.scale = w / item.width
            } else {
                item.scale = h / item.height
            }
        }
    }

    function performLayout() {
        // exclude repeater from the list of items we are going to position
        var visualChildren = [];
        for(var i = 0; i < layout.children.length; ++i) {
            var child = layout.children[i];
            if(child.toString().indexOf('QQuickRepeater') === -1) {
                console.debug('child type: ', child.toString())
                visualChildren.push(child);
            }
        }

        if (visualChildren.length === 0)
            return

        switch(visualChildren.length) {
        case 1:
            fitToSpace(visualChildren[0], width, height)
            centerInCell3x3(1, 1, visualChildren[0]);
            break;
        case 2:
            fitToSpace(visualChildren[0], width / 3, height / 2)
            fitToSpace(visualChildren[1], width / 3, height / 2)
            centerInCell2x3(0, 1, visualChildren[0]);
            centerInCell2x3(1, 1, visualChildren[1]);
            break;
        case 3:
            fitToSpace(visualChildren[0], width / 3, height / 3)
            fitToSpace(visualChildren[1], width / 3, height / 3)
            fitToSpace(visualChildren[2], width / 3, height / 3)
            centerInCell3x3(0, 1, visualChildren[0]);
            centerInCell3x3(1, 1, visualChildren[1]);
            centerInCell3x3(2, 1, visualChildren[2]);
            break;
        case 4:
            fitToSpace(visualChildren[0], width / 2, height / 2)
            fitToSpace(visualChildren[1], width / 2, height / 2)
            fitToSpace(visualChildren[2], width / 2, height / 2)
            fitToSpace(visualChildren[3], width / 2, height / 2)
            centerInCell2x2(0, 0, visualChildren[0]);
            centerInCell2x2(0, 1, visualChildren[1]);
            centerInCell2x2(1, 0, visualChildren[2]);
            centerInCell2x2(1, 1, visualChildren[3]);
            break;
        case 5:
            fitToSpace(visualChildren[0], width / 2, height / 3)
            fitToSpace(visualChildren[1], width / 2, height / 3)
            fitToSpace(visualChildren[2], width / 2, height / 3)
            fitToSpace(visualChildren[3], width / 2, height / 3)
            fitToSpace(visualChildren[4], width / 2, height / 3)
            centerInCell3x2(0, 0, visualChildren[0]);
            centerInCell3x2(0, 1, visualChildren[1]);
            centerInCell3x3(1, 1, visualChildren[2]);
            centerInCell3x2(2, 0, visualChildren[3]);
            centerInCell3x2(2, 1, visualChildren[4]);
            break;
        case 6:
            fitToSpace(visualChildren[0], width / 2, height / 3)
            fitToSpace(visualChildren[1], width / 2, height / 3)
            fitToSpace(visualChildren[2], width / 2, height / 3)
            fitToSpace(visualChildren[3], width / 2, height / 3)
            fitToSpace(visualChildren[4], width / 2, height / 3)
            fitToSpace(visualChildren[5], width / 2, height / 3)
            centerInCell3x2(0, 0, visualChildren[0]);
            centerInCell3x2(0, 1, visualChildren[1]);
            centerInCell3x2(1, 0, visualChildren[2]);
            centerInCell3x2(1, 1, visualChildren[3]);
            centerInCell3x2(2, 0, visualChildren[4]);
            centerInCell3x2(2, 1, visualChildren[5]);
            break;
        case 7:
            fitToSpace(visualChildren[0], width / 3, height / 3)
            fitToSpace(visualChildren[1], width / 3, height / 3)
            fitToSpace(visualChildren[2], width / 3, height / 3)
            fitToSpace(visualChildren[3], width / 3, height / 3)
            fitToSpace(visualChildren[4], width / 3, height / 3)
            fitToSpace(visualChildren[5], width / 3, height / 3)
            fitToSpace(visualChildren[6], width / 3, height / 3)
            centerInCell3x3(0, 0, visualChildren[0]);
            centerInCell3x3(0, 1, visualChildren[1]);
            centerInCell3x3(0, 2, visualChildren[2]);
            centerInCell3x3(1, 0, visualChildren[3]);
            centerInCell3x3(1, 1, visualChildren[4]);
            centerInCell3x3(1, 2, visualChildren[5]);
            centerInCell3x3(2, 1, visualChildren[6]);
            break;
        case 8:
            fitToSpace(visualChildren[0], width / 3, height / 3)
            fitToSpace(visualChildren[1], width / 3, height / 3)
            fitToSpace(visualChildren[2], width / 3, height / 3)
            fitToSpace(visualChildren[3], width / 3, height / 3)
            fitToSpace(visualChildren[4], width / 3, height / 3)
            fitToSpace(visualChildren[5], width / 3, height / 3)
            fitToSpace(visualChildren[6], width / 3, height / 3)
            fitToSpace(visualChildren[7], width / 3, height / 3)
            centerInCell3x3(0, 0, visualChildren[0]);
            centerInCell3x3(0, 1, visualChildren[1]);
            centerInCell3x3(0, 2, visualChildren[2]);
            centerInCell3x3(1, 0, visualChildren[3]);
            centerInCell3x3(1, 1, visualChildren[4]);
            centerInCell3x3(1, 2, visualChildren[5]);
            centerInCell3x3(2, 0, visualChildren[6]);
            centerInCell3x3(2, 2, visualChildren[7]);
            break;
        case 9:
            fitToSpace(visualChildren[0], width / 3, height / 3)
            fitToSpace(visualChildren[1], width / 3, height / 3)
            fitToSpace(visualChildren[2], width / 3, height / 3)
            fitToSpace(visualChildren[3], width / 3, height / 3)
            fitToSpace(visualChildren[4], width / 3, height / 3)
            fitToSpace(visualChildren[5], width / 3, height / 3)
            fitToSpace(visualChildren[6], width / 3, height / 3)
            fitToSpace(visualChildren[7], width / 3, height / 3)
            fitToSpace(visualChildren[8], width / 3, height / 3)
            centerInCell3x3(0, 0, visualChildren[0]);
            centerInCell3x3(0, 1, visualChildren[1]);
            centerInCell3x3(0, 2, visualChildren[2]);
            centerInCell3x3(1, 0, visualChildren[3]);
            centerInCell3x3(1, 1, visualChildren[4]);
            centerInCell3x3(1, 2, visualChildren[5]);
            centerInCell3x3(2, 0, visualChildren[6]);
            centerInCell3x3(2, 1, visualChildren[7]);
            centerInCell3x3(2, 2, visualChildren[8]);
            break;
        }
    }

    function centerInCell(j, i, item, vCells, hCells) {
        var cellWidth = width / hCells
        var cellHeight = height / vCells

        item.x = (i * cellWidth) + (cellWidth - item.width) / 2
        item.y = (j * cellHeight) + (cellHeight - item.height) / 2
    }

    function centerInCell3x3(j, i, item) {
        centerInCell(j, i, item, 3, 3)
    }

    function centerInCell3x2(j, i, item) {
        centerInCell(j, i, item, 3, 2)
    }
	
    function centerInCell2x3(j, i, item) {
        centerInCell(j, i, item, 2, 3)
    }

    function centerInCell2x2(j, i, item) {
        centerInCell(j, i, item, 2, 2)
    }
}
