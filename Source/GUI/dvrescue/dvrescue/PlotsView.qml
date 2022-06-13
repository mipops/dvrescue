import QtQuick 2.0
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QwtQuick2 1.0
import SplitView 1.0

Rectangle {
    id: root
    color: "transparent"
    property alias evenVideoCurve: evenVideoCurve
    property alias oddVideoCurve: oddVideoCurve
    property alias evenAudioCurve: evenAudioCurve
    property alias oddAudioCurve: oddAudioCurve
    property vector2d overlay: Qt.vector2d(-1, -1)
    property real overlayOpacity: 0.25
    property color overlayColor: 'purple'

    property int startFrame: 0
    onStartFrameChanged: {
        console.debug('Plots: startFrame = ', startFrame)
        zoomAll();
    }

    property int endFrame: dataModel.total - 1
    onEndFrameChanged: {
        console.debug('Plots: endFrame = ', endFrame)
        zoomAll();
    }

    signal pickerMoved(int displayX, int plotX);
    signal markerClicked(int frameIndex);

    function zoomAll() {
        zoomAllButton.clicked();
    }

    property int markerHeight: 30
    property int markerMargin: 4
    property int framePos: -1
    property date updateTrigger: scroll.position, videoPlot.canvasItem.width, videoPlot.xBottomAxisRange, new Date()

    MarkersView {
        id: recMarkers

        height: markerHeight + markerMargin
        updateTrigger: root.updateTrigger

        anchors.left: parent.left
        anchors.leftMargin: videoPlot.canvasItem.x
        anchors.top: parent.top
        clip: true
        width: videoPlot.canvasItem.width
        z: 10
    }

    Connections {
        target: dataModel
        onUpdated: {
            var markers = dataModel.getMarkers();
            console.debug('markers: ', JSON.stringify(markers, 0, 4))

            recMarkers.markersModel.clear();
            tc_nMarkers.markersModel.clear();
            for(var i = 0; i < markers.length; ++i) {
                var marker = markers[i];
                if(marker.type === 'rec') {
                    recMarkers.markersModel.append(marker);
                }
                else if(marker.type === 'tc_n') {
                    tc_nMarkers.markersModel.append(marker);
                }
            }
        }
    }


    SplitView {
        id: plotsSplitView

        anchors.top: recMarkers.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: scrollLayout.top

        orientation: Qt.Vertical

        onHeightChanged: {
            videoLayout.height = plotsSplitView.height / 5 * 3.5
        }

        Item {
            id: videoLayout

            Component.onCompleted: {
                SplitView.preferredHeight = Qt.binding(function() { return height })
            }

            RowLayout {
                id: videoRow
                anchors.fill: videoLayout
                anchors.bottomMargin: markerHeight + markerMargin

                QwtQuick2Plot {
                    id: videoPlot
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    canvasItem.clip: true
                    xBottomAxisTitle: "frames, N"
                    xBottomAxisColor: "darkgray"
                    xBottomAxisEnabled: false
                    yLeftAxisTitle: "video error concealment (%)"
                    yLeftAxisColor: "darkgray"

                    Component.onCompleted: {
                        yLeftAxisFont.bold = false
                        yLeftAxisFont.pixelSize = yLeftAxisFont.pixelSize - 2
                        xBottomAxisFont.bold = false
                        xBottomAxisFont.pixelSize = xBottomAxisFont.pixelSize - 2
                    }

                    Rectangle {
                        parent: videoPlot.canvasItem
                        width: 1
                        height: parent.height
                        color: 'purple'
                        x: videoPlot.xBottomAxisRange, videoPlotPicker.transformPoint(Qt.point(framePos, 0)).x
                    }

                    Rectangle {
                        color: overlayColor
                        opacity: overlayOpacity
                        parent: videoPlot.canvasItem
                        height: parent.height
                        width: videoPlot.xBottomAxisRange, videoPlotPicker.transformPoint(Qt.point(overlay.y - overlay.x, 0)).x
                        x: videoPlot.xBottomAxisRange, videoPlotPicker.transformPoint(Qt.point(overlay.x, 0)).x
                        visible: overlay != Qt.vector2d(-1, -1)
                    }

                    PlotPicker {
                        id: videoPlotPicker
                        visible: dataModel.total !== 0
                        overlayTextFormatter: function(p) {
                            return dataModel.videoInfo(p.x, p.y);
                        }
                        onXChanged: if(active && visible) pickerMoved(x, point.x)
                        onActiveChanged: if(active && visible) pickerMoved(x, point.x)
                        onZoomed: scroll.setCustomZoom(x1, x2)
                        onMoved: scroll.move(x1)
                    }

                    QwtQuick2PlotCurve {
                        id: evenVideoCurve
                        title: "even";
                        curveStyle: QwtQuick2PlotCurve.Sticks
                        color: "darkgreen"
                        titleColor: "darkgray"
                    }

                    QwtQuick2PlotCurve {
                        id: oddVideoCurve
                        title: "odd";
                        curveStyle: QwtQuick2PlotCurve.Sticks
                        color: "green"
                        titleColor: "darkgray"
                    }

                    QwtQuick2PlotGrid {
                        enableXMin: true
                        enableYMin: true
                        majorPenColor: 'darkGray'
                        majorPenStyle: Qt.DotLine
                        minorPenColor: 'gray'
                        minorPenStyle: Qt.DotLine
                    }
                }

                QwtQuick2PlotLegend {
                    plotItem: videoPlot
                    width: 50
                    height: 50
                }
            }

            MarkersView {
                anchors.top: videoRow.bottom
                anchors.left: parent.left
                anchors.leftMargin: videoPlot.canvasItem.x
                id: tc_nMarkers
                imageRotation: 180
                enableColorize: true
                colorizeColor: 'orange'

                height: markerHeight + markerMargin
                updateTrigger: root.updateTrigger
                tooltipFormatter: function(name, timecode, recordingTime, frameNumber) {
                    return name + " " + "to ${tc}, Recording Time = ${rdt}".replace("${tc}", timecode).replace("${rdt}", recordingTime) + ", Frame: " + frameNumber
                }

                clip: true
                width: videoPlot.canvasItem.width
                z: 10
            }
        }

        RowLayout {
            id: audioLayout

            anchors.topMargin: 5

            QwtQuick2Plot {
                id: audioPlot
                Layout.fillWidth: true
                Layout.fillHeight: true

                canvasItem.clip: true
                xBottomAxisTitle: "frames, N"
                xBottomAxisColor: "darkgray"
                yLeftAxisTitle: "audio error (%)"
                yLeftAxisColor: "darkgray"

                Component.onCompleted: {
                    yLeftAxisFont.bold = false
                    yLeftAxisFont.pixelSize = yLeftAxisFont.pixelSize - 2
                    xBottomAxisFont.bold = false
                    xBottomAxisFont.pixelSize = xBottomAxisFont.pixelSize - 2
                }

                Rectangle {
                    parent: audioPlot.canvasItem
                    width: 1
                    height: parent.height
                    color: 'purple'
                    x: audioPlot.xBottomAxisRange, audioPlotPicker.transformPoint(Qt.point(framePos, 0)).x
                }

                Rectangle {
                    color: overlayColor
                    opacity: overlayOpacity
                    parent: audioPlot.canvasItem
                    height: parent.height
                    width: audioPlot.xBottomAxisRange, audioPlotPicker.transformPoint(Qt.point(overlay.y - overlay.x, 0)).x
                    x: audioPlot.xBottomAxisRange, audioPlotPicker.transformPoint(Qt.point(overlay.x, 0)).x
                    visible: overlay != Qt.vector2d(-1, -1)
                }

                PlotPicker {
                    id: audioPlotPicker
                    visible: dataModel.total !== 0
                    overlayTextFormatter: function(p) {
                        return dataModel.audioInfo(p.x, p.y);
                    }
                    onXChanged: if(active && visible) pickerMoved(x, point.x)
                    onActiveChanged: if(active && visible) pickerMoved(x, point.x)
                    onZoomed: scroll.setCustomZoom(x1, x2)
                    onMoved: scroll.move(x1)
                }

                QwtQuick2PlotCurve {
                    id: evenAudioCurve
                    title: "even";
                    curveStyle: QwtQuick2PlotCurve.Sticks
                    color: "darkblue"
                    titleColor: "darkgray"
                }

                QwtQuick2PlotCurve {
                    id: oddAudioCurve
                    title: "odd"
                    curveStyle: QwtQuick2PlotCurve.Sticks
                    color: "blue"
                    titleColor: "darkgray"
                }

                QwtQuick2PlotGrid {
                    enableXMin: true
                    enableYMin: true
                    majorPenColor: 'darkGray'
                    majorPenStyle: Qt.DotLine
                    minorPenColor: 'gray'
                    minorPenStyle: Qt.DotLine
                }
            }

            QwtQuick2PlotLegend {
                plotItem: audioPlot
                width: 50
                height: 50
            }
        }
    }

    RowLayout {
        id: scrollLayout
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: zoomInButton.height
        property int zoomFactor: 2

        CustomButton {
            id: zoomInButton
            icon.source: "/icons/button-zoom-in.svg"

            onClicked: {
                var newRight = videoPlot.xBottomAxisRange.x + (videoPlot.xBottomAxisRange.y - videoPlot.xBottomAxisRange.x) / scrollLayout.zoomFactor;
                var rangeCount = newRight - videoPlot.xBottomAxisRange.x + 1
                scroll.size = rangeCount / (endFrame - startFrame + 1)

                videoPlot.xBottomAxisRange = Qt.vector2d(videoPlot.xBottomAxisRange.x, newRight)
                audioPlot.xBottomAxisRange = videoPlot.xBottomAxisRange
            }
        }
        CustomButton {
            id: zoomAllButton
            icon.source: "/icons/button-fit-to-screen.svg"

            onClicked: {
                scroll.size = 1
                scroll.position = 0
                videoPlot.xBottomAxisRange = Qt.vector2d(startFrame, endFrame)
                audioPlot.xBottomAxisRange = videoPlot.xBottomAxisRange
            }
        }

        CustomButton {
            id: customZoomButton
            icon.source: "/icons/button-custom-range.svg"

            Connections {
                target: dataModel
                onPopulated: {
                    customZoomSelector.x1 = videoPlot.xBottomAxisRange.x
                    customZoomSelector.x2 = videoPlot.xBottomAxisRange.y
                }
            }

            onClicked: {
                customZoomSelector.open();
            }

            Dialog {
                id: customZoomSelector
                x: Math.round((parent.width - width) / 2)
                y: - height

                property alias x1: x1TextField.text
                property alias x2: x2TextField.text

                onVisibleChanged: {
                    if(visible)
                        x1TextField.forceActiveFocus();
                }

                function apply() {
                    var left = Math.min(Number(customZoomSelector.x1), Number(customZoomSelector.x2));
                    var right = Math.max(Number(customZoomSelector.x1), Number(customZoomSelector.x2));

                    if(left === right && left === 0)
                        return;

                    scroll.setCustomZoom(left, right);
                    customZoomSelector.close();
                }

                ColumnLayout {
                    Keys.onEnterPressed: customZoomSelector.apply();
                    Keys.onReturnPressed: customZoomSelector.apply();

                    RowLayout {
                        TextField {
                            id: x1TextField
                            validator: IntValidator {}
                            selectByMouse: true
                            placeholderText: "x1"
                        }
                        TextField {
                            id: x2TextField
                            validator: IntValidator {}
                            selectByMouse: true
                            placeholderText: "x2"
                        }
                    }

                    Button {
                        Layout.alignment: Qt.AlignHCenter
                        text: "Apply"

                        onClicked: customZoomSelector.apply();
                    }
                }
            }
        }

        CustomButton {
            id: zoomOutButton
            icon.source: "/icons/button-zoom-out.svg"

            onClicked: {
                var newRight = videoPlot.xBottomAxisRange.x + (videoPlot.xBottomAxisRange.y - videoPlot.xBottomAxisRange.x) * scrollLayout.zoomFactor;
                var rangeCount = newRight - videoPlot.xBottomAxisRange.x + 1

                var originalPosition = videoPlot.xBottomAxisRange.x + (videoPlot.xBottomAxisRange.y - videoPlot.xBottomAxisRange.x) * scroll.position / (1 - scroll.size)
                var originalScale = scroll.size
                var originalRange = videoPlot.xBottomAxisRange.y - videoPlot.xBottomAxisRange.x + 1

                // console.debug('originalPosition: ', originalPosition, 'scroll.position: ', scroll.position, 'scroll.size: ', scroll.size)

                scroll.size = rangeCount / (endFrame - startFrame + 1);

                var newLeft = videoPlot.xBottomAxisRange.x;
                var needPositionChange = false;

                if(newRight > (endFrame))
                {
                    newLeft = Math.max(startFrame, newLeft - (newRight - (endFrame)));
                    newRight = (endFrame)

                    needPositionChange = true;
                }

                var newRange = Qt.vector2d(newLeft, newRight);

                videoPlot.xBottomAxisRange = newRange
                audioPlot.xBottomAxisRange = videoPlot.xBottomAxisRange

                if(needPositionChange) {
                    // console.debug('originalPosition: ', originalPosition, 'scroll.position: ', scroll.position)
                    scroll.position = Math.max(0, (originalPosition - videoPlot.xBottomAxisRange.x) * (1 - scroll.size) / (videoPlot.xBottomAxisRange.y - videoPlot.xBottomAxisRange.x))
                }
            }
        }

        Text {
            color: "white"
            Layout.minimumWidth: 40
            horizontalAlignment: Text.AlignHCenter
            text: videoPlot.xBottomAxisRange.x
        }

        ScrollBar {
            id: scroll
            orientation: "Horizontal"
            height: parent.height
            hoverEnabled: true
            active: true
            policy: ScrollBar.AlwaysOn
            Layout.fillWidth: true

            background: Rectangle {
                implicitWidth: scroll.interactive ? 16 : 4
                implicitHeight: scroll.interactive ? 16 : 4
                color: "white"
                opacity: 0.0
                visible: scroll.interactive
            }

            /*
            WheelHandler {
                onWheel: {
                    if(event.angleDelta.y > 0)
                        scroll.increase();
                    else if(event.angleDelta.y < 0)
                        scroll.decrease();
                }
            }
            */

            function move(x1) {
                var rangeCount = Math.round(videoPlot.xBottomAxisRange.y) - Math.round(videoPlot.xBottomAxisRange.x) + 1
                var newPos = Math.max(startFrame, x1 / (endFrame - startFrame + 1));

                if(newPos < 0)
                    newPos = 0;
                if(newPos > (1 - size))
                    newPos = 1 - size;

                scroll.position = newPos
            }

            function setCustomZoom(x1, x2) {
                var rangeCount = x2 - x1 + 1
                scroll.size = rangeCount / (endFrame - startFrame + 1)

                videoPlot.xBottomAxisRange = Qt.vector2d(x1, x2)
                audioPlot.xBottomAxisRange = videoPlot.xBottomAxisRange

                scroll.position = Math.max(startFrame, x1 / (endFrame - startFrame + 1))
            }

            stepSize: size / 100

            onPositionChanged: {
                console.debug('position changed: ', position)

                var rangeCount = Math.round(videoPlot.xBottomAxisRange.y) - Math.round(videoPlot.xBottomAxisRange.x) + 1
                var from = Math.round(position * (endFrame - startFrame + 1));
                var to = Math.round(Math.min((endFrame - startFrame + 1), from + rangeCount)) - 1

                // console.debug('from: ', from, 'to: ', to, 'rangeCount: ', rangeCount, 'to - from: ', to - from)

                videoPlot.xBottomAxisRange = Qt.vector2d(Math.round(to - rangeCount + 1), Math.round(to));
                // console.debug('new rangeCount: ', Math.round(videoPlot.xBottomAxisRange.y) - Math.round(videoPlot.xBottomAxisRange.x),
                // 'from: ', videoPlot.xBottomAxisRange.x, 'to: ', videoPlot.xBottomAxisRange.y)

                audioPlot.xBottomAxisRange = Qt.vector2d(Math.round(to - rangeCount + 1), Math.round(to));
            }
        }

        Text {
            color: "white"
            Layout.minimumWidth: 40
            horizontalAlignment: Text.AlignHCenter
            text: videoPlot.xBottomAxisRange.y
        }
    }

}
