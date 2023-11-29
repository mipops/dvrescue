import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import TableModel 1.0
import TableModelColumn 1.0
import Qt.labs.qmlmodels 1.0

Dialog {
    id: root

    modal: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    parent: Overlay.overlay
    width: (image.source !== null && image.status === Image.Ready ? image.paintedWidth : 100) + padding * 2
    height: (image.source != null && image.status === Image.Ready ? image.paintedHeight : 100) + padding * 2 + 20
    clip: true
    padding: 5
    spacing: 0
    property bool showBusyIndicator: false

    onClosed: {
        imageSource = null
    }

    property string imageSource

    Image {
        id: image
        x: -(width - paintedWidth) / 2
        y: -(height - paintedHeight) / 2

        width: image.sourceSize.width
        height: image.sourceSize.height
        fillMode: Image.PreserveAspectFit
        source: imageSource
        onStatusChanged: {
            if(status === Image.Ready) {
                if(image.sourceSize.width > root.parent.width)
                    image.width = root.parent.width * 0.9
                if(image.sourceSize.height > root.parent.height)
                    image.height = root.parent.height * 0.9
            }
        }
    }

    BusyIndicator {
        visible: showBusyIndicator
        anchors.centerIn: parent
    }

    ResizeRectangle {
        target: root
        anchors.fill: root.contentItem
        anchors.margins: -root.padding
    }
}


