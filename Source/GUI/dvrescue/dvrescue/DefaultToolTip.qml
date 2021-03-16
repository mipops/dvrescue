import QtQuick 2.0
import QtQuick.Controls 2.12

ToolTip {
    id: control
    delay: 100
    timeout: 3000

    enter: Transition {
        // toast_enter
        NumberAnimation { property: "opacity"; from: 0.0; to: 1.0; easing.type: Easing.OutQuad; duration: 100 }
    }

    exit: Transition {
        // toast_exit
        NumberAnimation { property: "opacity"; from: 1.0; to: 0.0; easing.type: Easing.InQuad; duration: 100 }
    }

    Component.onCompleted: {
        font.pointSize = font.pointSize - 1
    }
}
