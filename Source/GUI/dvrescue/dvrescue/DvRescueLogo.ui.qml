import QtQuick 2.12
import QtQuick.Layouts 1.11
import QtQuick.Controls 2.15

Rectangle {
    width: 162
    height: 100
    color: "#ffffff"

    BorderImage {
        id: borderImage
        anchors.centerIn: parent

        width: 132
        height: 92
        source: "dvrescue.png"
        verticalTileMode: BorderImage.Round
        horizontalTileMode: BorderImage.Round
        border.bottom: 3
        border.top: 3
        border.right: 3
        border.left: 3
    }
}
