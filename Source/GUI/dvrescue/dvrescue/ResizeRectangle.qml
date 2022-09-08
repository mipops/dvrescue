import QtQuick 2.0

Item {
    id: resizeRectangle

    property var target: parent
    property bool forceSquare: false

    ResizeHandle {
        id: topLeft
        target: resizeRectangle.target
        anchors {
            left: parent.left
            top: parent.top
            leftMargin: -width / 2
            topMargin: -height / 2
        }
        cursorShape: Qt.SizeFDiagCursor
        xFactor: 1.0
        yFactor: 1.0
        widthFactor: -1.0
        heightFactor: -1.0
        forceSquare: resizeRectangle.forceSquare
    }

    ResizeHandle {
        id: left
        target: resizeRectangle.target
        anchors {
            top: topLeft.bottom
            bottom: bottomLeft.top
            leftMargin: -width / 2
        }
        cursorShape: Qt.SizeHorCursor
        xFactor: 1.0
        yFactor: 0
        widthFactor: -1.0
        heightFactor: 0
        forceSquare: resizeRectangle.forceSquare
    }

    ResizeHandle {
        id: bottomLeft
        target: resizeRectangle.target
        anchors {
            left: parent.left
            bottom: parent.bottom
            leftMargin: -width / 2
            bottomMargin: -height / 2
        }
        cursorShape: Qt.SizeBDiagCursor
        xFactor: 1.0
        yFactor: 0.0
        widthFactor: -1.0
        heightFactor: 1.0
        forceSquare: resizeRectangle.forceSquare
    }

    ResizeHandle {
        id: bottom
        target: resizeRectangle.target
        anchors {
            bottom: bottomLeft.bottom
            left: bottomLeft.right
            right: bottomRight.left
            bottomMargin: -height / 2
        }
        cursorShape: Qt.SizeVerCursor
        xFactor: 0
        yFactor: 0
        widthFactor: 0
        heightFactor: 1.0
        forceSquare: resizeRectangle.forceSquare
    }

    ResizeHandle {
        id: bottomRight
        anchors {
            right: parent.right
            bottom: parent.bottom
            rightMargin: -width / 2
            bottomMargin: -height / 2
        }
        cursorShape: Qt.SizeFDiagCursor
        target: resizeRectangle.target
        xFactor: 0.0
        yFactor: 0.0
        widthFactor:  1.0
        heightFactor: 1.0
        forceSquare: resizeRectangle.forceSquare
    }

    ResizeHandle {
        id: right
        anchors {
            right: parent.right
            bottom: bottomRight.top
            top: topRight.bottom
            rightMargin: -width / 2
        }
        cursorShape: Qt.SizeHorCursor
        target: resizeRectangle.target
        xFactor: 0.0
        yFactor: 0.0
        widthFactor:  1.0
        heightFactor: 0
        forceSquare: resizeRectangle.forceSquare
    }

    ResizeHandle {
        id: topRight
        anchors {
            right: parent.right
            top: parent.top
            rightMargin: -width / 2
            topMargin: -height / 2
        }
        cursorShape: Qt.SizeBDiagCursor
        target: resizeRectangle.target
        xFactor: 0.0
        yFactor: 1.0
        widthFactor: 1.0
        heightFactor: -1.0
        forceSquare: resizeRectangle.forceSquare
    }

    ResizeHandle {
        id: top
        anchors {
            right: topRight.left
            left: topLeft.right
            topMargin: -height / 2
        }
        cursorShape: Qt.SizeVerCursor
        target: resizeRectangle.target
        xFactor: 0.0
        yFactor: 1.0
        widthFactor: 0
        heightFactor: -1.0
        forceSquare: resizeRectangle.forceSquare
    }
}
