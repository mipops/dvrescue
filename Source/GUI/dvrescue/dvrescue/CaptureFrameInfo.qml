import QtQuick 2.0
import QtQuick.Layouts 1.12

GridLayout {
    property int fontPixlSize: 30
    property string frameNumber: "n/a"
    property string timeCode: "n/a"
    property string recDate: "n/a"
    property string recTime: "n/a"

    LCDLabel {
        Layout.column: 0
        Layout.row: 0
        Layout.minimumWidth: parent.width / 4

        font.pixelSize: fontPixlSize
        text: frameNumber
        color: "green"
    }

    LCDLabel {
        Layout.column: 0
        Layout.row: 1
        Layout.minimumWidth: parent.width / 4

        font.pixelSize: fontPixlSize
        text: "FRAME#"
    }

    LCDLabel {
        Layout.column: 1
        Layout.row: 0
        Layout.minimumWidth: parent.width / 4

        font.pixelSize: fontPixlSize
        text: timeCode
        color: "green"
    }

    LCDLabel {
        Layout.column: 1
        Layout.row: 1
        Layout.minimumWidth: parent.width / 4

        font.pixelSize: fontPixlSize
        text: "TIMECODE"
    }

    LCDLabel {
        Layout.column: 2
        Layout.row: 0
        Layout.minimumWidth: parent.width / 2

        font.pixelSize: fontPixlSize
        text: recTime
        color: "green"
    }

    LCDLabel {
        Layout.column: 2
        Layout.row: 1
        Layout.minimumWidth: parent.width / 2

        font.pixelSize: fontPixlSize
        text: "REC TIME"
    }
}
