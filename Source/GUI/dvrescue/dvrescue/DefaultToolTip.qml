import QtQuick 2.0
import QtQuick.Controls 2.12

ToolTip {
    id: control
    delay: 500
    timeout: 3000

    Component.onCompleted: {
        font.pointSize = font.pointSize - 1
    }
}
