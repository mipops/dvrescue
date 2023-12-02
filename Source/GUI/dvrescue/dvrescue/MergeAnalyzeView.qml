import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import TableModel 1.0
import TableModelColumn 1.0
import Qt.labs.qmlmodels 1.0

Dialog {
    id: root

    property bool canPrev: true
    property bool canNext: true
    signal prev();
    signal next();
    signal refresh();

    property alias frameNumberText: frameNumber.text
    property alias timecodeText: timecode.text

    modal: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    parent: Overlay.overlay
    width: (paintedWidth !== 0 ? image.paintedWidth : 100) + padding * 2
    height: (paintedHeight !== 0 ? image.paintedHeight + buttonsLayout.height : 100) + padding * 2 + 20
    clip: true
    padding: 5
    spacing: 0
    property bool showBusyIndicator: false

    onClosed: {
        imageSource = null
    }

    property string imageSource

    property int paintedWidth: 0
    property int paintedHeight: 0

    RowLayout {
        id: buttonsLayout
        visible: root.paintedWidth !== 0

        anchors.horizontalCenter: parent.horizontalCenter
        Layout.alignment: Qt.AlignHCenter

        Button {
            text: "<"
            enabled: canPrev
            onClicked: prev()
        }

        Label {
            text: "Frame #"
        }

        Label {
            id: frameNumber
            text: ''
            Layout.minimumWidth: 50
        }

        Label {
            text: "Timecode"
        }

        Label {
            id: timecode
            text: ''
            Layout.minimumWidth: 50
        }

        Button {
            text: ">"
            enabled: canNext
            onClicked: next()
        }
    }

    Image {
        id: image
        x: -(width - paintedWidth) / 2
        y: -(height - paintedHeight) / 2 + buttonsLayout.height

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
        onPaintedWidthChanged: {
            if(root.paintedWidth === 0)
                root.paintedWidth = paintedWidth
        }
        onPaintedHeightChanged: {
            if(root.paintedHeight === 0)
                root.paintedHeight = paintedHeight
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


