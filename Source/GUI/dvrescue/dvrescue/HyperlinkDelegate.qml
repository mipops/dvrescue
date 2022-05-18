import QtQuick 2.0
import QtQuick.Controls 2.12
import QtQuick 2.12

Rectangle {
    id: textDelegate

    signal linkActivated(string link);
    signal clicked();

    implicitWidth: 100
    implicitHeight: 20
    color: (row % 2) == 0 ? 'gray' : 'lightgray'
    property real overlayColorOpacity: 0.5
    property alias overlayColor: overlay.color
    property alias overlayVisible: overlay.visible
    property alias text: textLabel.text
    property alias textFont: textLabel.font

    MouseArea {
        anchors.fill: parent
        onClicked: {
            textDelegate.clicked()
        }
    }

    Text {
        id: textLabel
        text: display
        anchors.centerIn: parent
        textFormat: Text.StyledText
        linkColor: 'blue'
        onLinkActivated: {
            textDelegate.linkActivated(link)
        }

        MouseArea {
            anchors.fill: textLabel
            cursorShape: textLabel.hoveredLink ? Qt.PointingHandCursor : Qt.ArrowCursor
            acceptedButtons: Qt.NoButton
        }
    }

    Rectangle {
        id: overlay
        anchors.fill: parent
        opacity: overlayColorOpacity
        visible: false
    }
}
