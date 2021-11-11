import QtQuick 2.12
import QtQuick.Layouts 1.11
import QtQuick.Controls 2.12
import QtGraphicalEffects 1.0

Button {
    id: button
    width: 71
    height: 71
    padding: 0
    topInset: 0
    leftInset: 0
    bottomInset: 0
    rightInset: 0

    icon.width: width
    icon.height: height

    background: Rectangle {
        id: buttonBackground
        color: 'transparent'
    }

    Glow {
        id: glow
        anchors.fill: button.contentItem
        radius: 4
        samples: 17
        color: "gray"
        source: button.contentItem
        visible: button.pressed && !button.checked
    }

    DropShadow {
        anchors.fill: button.contentItem
        horizontalOffset: 3
        verticalOffset: 3
        radius: 8.0
        samples: 17
        color: "#80000000"
        source: button.contentItem
        visible: button.hovered
    }
}
