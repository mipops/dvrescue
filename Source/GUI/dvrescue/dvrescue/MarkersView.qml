import QtQuick 2.0
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 1.4 as QQC1
import QwtQuick2 1.0
import QtGraphicalEffects 1.0

Item {
    id: markersView
    /* + videoPlotPicker.transform(Qt.point(0, 0)).x */
    /* - videoPlotPicker.transform(Qt.point(0, 0)).x */

    property alias markersModel: markersModel
    property date updateTrigger: new Date()
    property int imageRotation: 0
    property color colorizeColor: 'transparent'
    property bool enableColorize: false;
    property var tooltipFormatter;

    Repeater {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        delegate: Item {
            y: markerMargin / 2
            x: updateTrigger, videoPlotPicker.transform(Qt.point(frameNumber, 0)).x - width / 2 /*- videoPlotPicker.transform(Qt.point(0, 0)).x*/
            height: image.height
            width: image.width
            z: mouseTracker.containsMouse ? 2 : (frameNumber === framePos ? 1 : 0)

            Image {
                id: image
                rotation: imageRotation
                source: icon
                height: markerHeight
                fillMode: Image.PreserveAspectFit
                visible: true
            }

            ColorOverlay {
                id: colorOverlay
                anchors.fill: image
                color: colorizeColor
                rotation: imageRotation
                source: image
                visible: enableColorize
            }

            Glow {
                id: glow
                anchors.fill: image
                radius: 4
                samples: 17
                rotation: imageRotation
                color: mouseTracker.containsMouse || frameNumber === framePos ? "white" : "gray"
                source: colorOverlay.visible ? colorOverlay : image
                visible: true
            }

            DropShadow {
                anchors.fill: glow
                horizontalOffset: 3
                verticalOffset: 3
                rotation: imageRotation
                radius: 8.0
                samples: 17
                color: "#80000000"
                source: glow
                visible: mouseTracker.containsMouse
            }

            DefaultToolTip {
                visible: mouseTracker.containsMouse
                text: tooltipFormatter ? tooltipFormatter(name, timecode, recordingTime, frameNumber)
                                       : name + " " + "Timecode: " + timecode + ", Recording Time: " + recordingTime + ", Frame: " + frameNumber
            }

            MouseArea {
                id: mouseTracker
                anchors.fill: image
                hoverEnabled: true
                onClicked: {
                    markerClicked(frameNumber)
                }
            }
        }

        model: ListModel {
            id: markersModel
        }
    }
}
