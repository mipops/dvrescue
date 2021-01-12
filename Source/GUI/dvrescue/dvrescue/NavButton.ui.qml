import QtQuick 2.12
import QtQuick.Layouts 1.11
import QtQuick.Controls 2.15

Rectangle {
    id: rectangle
    width: 162
    height: 80
    property string text: "Button"
    property alias textColor: buttonText.color
    color: "#a40000"

    Text {
        id: buttonText
        anchors.centerIn: parent
        text: parent.text
        color: parent.textColor
        font.pixelSize: 26
        font.family: "Tahoma"
        minimumPixelSize: 14
        font.weight: Font.Bold
        minimumPointSize: 14
    }
}
