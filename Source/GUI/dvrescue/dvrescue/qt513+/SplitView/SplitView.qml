import QtQuick.Controls 2.13 as QQC2
import QtQuick 2.12

QQC2.SplitView {
    id: control

    property int preferredWidth: 0
    onPreferredWidthChanged: {
        SplitView.preferredWidth = preferredWidth
    }

    property int preferredHeight: 0
    onPreferredHeightChanged: {
        SplitView.preferredHeight = preferredHeight
    }

    handle: Rectangle {
        implicitWidth: control.orientation === Qt.Horizontal ? 3 : control.width
        implicitHeight: control.orientation === Qt.Horizontal ? control.height : 3

        Rectangle {
            anchors.topMargin: control.orientation == Qt.Vertical ? 1 : 0
            anchors.bottomMargin: control.orientation == Qt.Vertical ? 1 : 0
            anchors.leftMargin: control.orientation == Qt.Horizontal ? 1 : 0
            anchors.rightMargin: control.orientation == Qt.Horizontal ? 1 : 0
            anchors.fill: parent
            color: 'lightgray'
        }

        color: "white"
    }
}
