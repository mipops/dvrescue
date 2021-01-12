import QtQuick 2.12
import QtQuick.Layouts 1.11
import QtQuick.Controls 2.15

Button {
    id: button
    width: 162
    height: 80
    property color textColor: "white"
    property color buttonColor: "black"
    padding: 0
    topInset: 0
    leftInset: 0
    bottomInset: 0
    rightInset: 0

    background: Rectangle {
        id: buttonBackground
        color: button.pressed ? Qt.darker(button.buttonColor) : button.buttonColor

        Item {
            anchors.left: parent.right
            anchors.leftMargin: -button.width / 3
            anchors.verticalCenter: parent.verticalCenter
            width: childrenRect.width
            height: childrenRect.height

            Rectangle {
                visible: button.checked
                transformOrigin: Item.Center
                color: buttonBackground.color
                width: button.width / 3
                height: width
                rotation: 45
            }
        }
    }

    contentItem: Text {
        id: buttonText
        anchors.centerIn: parent
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        text: button.text
        color: button.textColor
        font.pixelSize: 26
        font.family: "Tahoma"
        minimumPixelSize: 14
        font.weight: Font.Bold
        minimumPointSize: 14
    }
}
