import QtQuick 2.0

Item {
    id: topLeft

    property var target
    property bool ignoreChange: false

    property real xFactor: 0.0
    property real yFactor: 0.0
    property real widthFactor: 0.0
    property real heightFactor: 0.0

    property bool forceSquare: false
    property alias cursorShape: mouseArea.cursorShape

    width: 5
    height: 5

    MouseArea {
        id: mouseArea
        property point previousPosition
        anchors.fill: parent
        onPositionChanged: {
            var deltaX = mouse.x - previousPosition.x;
            var deltaY = mouse.y - previousPosition.y;

            var biggestIsX = Math.abs(deltaX) > Math.abs(deltaY);
            if(forceSquare) {
                if(biggestIsX) {
                    deltaX = widthFactor * heightFactor * deltaY;
                } else {
                    deltaY = widthFactor * heightFactor * deltaX;
                }
            }

            var newX = target.x + xFactor * deltaX;
            var xDiff = target.x - newX;
            target.x = newX;

            var newWidth = target.width + widthFactor * deltaX;
            if(newWidth > 32) {
                target.width = newWidth;
            }
            if(forceSquare && !biggestIsX) {
                previousPosition.y -= deltaY;
            }
            var newY = target.y + yFactor * deltaY;
            var yDiff = target.y - newY;
            target.y = newY;

            var newHeight = target.height + heightFactor * deltaY;
            if(newHeight > 32) {
                target.height = newHeight;
            }
            if(forceSquare && biggestIsX) {
                previousPosition.x -= deltaX;
            }
        }
        onPressed: {
            previousPosition.x = mouse.x
            previousPosition.y = mouse.y
        }
    }
}
