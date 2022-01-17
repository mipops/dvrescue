import QtQuick 2.0
import QtQuick.Layouts 1.12

GridLayout {
    property int fontPixlSize: 30
    property string frameNumber: "n/a"
    property string timeCode: "n/a"
    property string recDate: "n/a"
    property string recTime: "n/a"

    LCDLabel {
        GridLayout.column: 0
        GridLayout.row: 0
        GridLayout.alignment: Qt.AlignHCenter

        font.pixelSize: fontPixlSize
        text: frameNumber
        color: "green"
    }

    LCDLabel {
        GridLayout.column: 0
        GridLayout.row: 1
        GridLayout.alignment: Qt.AlignHCenter

        font.pixelSize: fontPixlSize
        text: "FRAME#"
    }

    LCDLabel {
        GridLayout.column: 1
        GridLayout.row: 0
        GridLayout.alignment: Qt.AlignHCenter

        font.pixelSize: fontPixlSize
        text: timeCode
        color: "green"
    }

    LCDLabel {
        GridLayout.column: 1
        GridLayout.row: 1
        GridLayout.alignment: Qt.AlignHCenter

        font.pixelSize: fontPixlSize
        text: "TIMECODE"
    }

    LCDLabel {
        GridLayout.column: 2
        GridLayout.row: 0
        GridLayout.alignment: Qt.AlignHCenter

        font.pixelSize: fontPixlSize
        text: recDate
        color: "green"
    }

    LCDLabel {
        GridLayout.column: 2
        GridLayout.row: 1
        GridLayout.alignment: Qt.AlignHCenter

        font.pixelSize: fontPixlSize
        text: "REC DATE"
    }

    LCDLabel {
        GridLayout.column: 3
        GridLayout.row: 0
        GridLayout.alignment: Qt.AlignHCenter

        font.pixelSize: fontPixlSize
        text: recTime
        color: "green"
    }

    LCDLabel {
        GridLayout.column: 3
        GridLayout.row: 1
        GridLayout.alignment: Qt.AlignHCenter

        font.pixelSize: fontPixlSize
        text: "REC TIME"
    }
}
