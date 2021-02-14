import QtQuick 2.0
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 1.4 as QQC1
import QwtQuick2 1.0

Rectangle {
    color: "transparent"
    property alias evenVideoCurve: evenVideoCurve
    property alias oddVideoCurve: oddVideoCurve
    property alias evenAudioCurve: evenAudioCurve
    property alias oddAudioCurve: oddAudioCurve

    signal pickerMoved(int displayX, int plotX);

    function zoomAll() {
        zoomAllButton.clicked();
    }

    property int framePos: -1

    QQC1.SplitView {
        id: plotsSplitView

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: scrollLayout.top

        orientation: Qt.Vertical

        onHeightChanged: {
            videoLayout.height = plotsSplitView.height / 5 * 3.5
        }

        RowLayout {
            id: videoLayout

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
                    color: 'red'
                    x: videoPlotPicker.transform(Qt.point(framePos, 0)).x
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
                    color: 'red'
                    x: audioPlotPicker.transform(Qt.point(framePos, 0)).x
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

        Button {
            id: zoomInButton
            implicitHeight: 38
            implicitWidth: 60
            background: Item {}
            icon.source: "icons/button-zoom-in.svg"
            icon.width: width
            icon.height: height
            icon.color: "white"
            padding: 0
            topInset: 0
            leftInset: 0
            bottomInset: 0
            rightInset: 0
            // text: "+"
            onClicked: {
                var newRight = videoPlot.xBottomAxisRange.x + (videoPlot.xBottomAxisRange.y - videoPlot.xBottomAxisRange.x) / scrollLayout.zoomFactor;
                var rangeCount = newRight - videoPlot.xBottomAxisRange.x + 1
                scroll.size = rangeCount / dataModel.total

                videoPlot.xBottomAxisRange = Qt.vector2d(videoPlot.xBottomAxisRange.x, newRight)
                audioPlot.xBottomAxisRange = videoPlot.xBottomAxisRange
            }
        }
        Button {
            id: zoomAllButton
            implicitHeight: 38
            implicitWidth: 60
            background: Item {}
            icon.source: "icons/button-fit-to-screen.svg"
            icon.width: width
            icon.height: height
            icon.color: "white"
            padding: 0
            topInset: 0
            leftInset: 0
            bottomInset: 0
            rightInset: 0
            // text: "|"

            onClicked: {
                scroll.size = 1
                scroll.position = 0
                videoPlot.xBottomAxisRange = Qt.vector2d(0, dataModel.total - 1)
                audioPlot.xBottomAxisRange = videoPlot.xBottomAxisRange
            }
        }

        Button {
            id: customZoomButton
            implicitHeight: 38
            implicitWidth: 60
            background: Item {}
            icon.source: "icons/button-custom-range.svg"
            icon.width: width
            icon.height: height
            icon.color: "white"
            padding: 0
            topInset: 0
            leftInset: 0
            bottomInset: 0
            rightInset: 0
            // text: "Custom"

            Connections {
                target: dataModel
                function onPopulated() {
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

        Button {
            id: zoomOutButton
            implicitHeight: 38
            implicitWidth: 60
            background: Item {}
            icon.source: "icons/button-zoom-out.svg"
            icon.width: width
            icon.height: height
            icon.color: "white"
            padding: 0
            topInset: 0
            leftInset: 0
            bottomInset: 0
            rightInset: 0
            // text: "-"
            onClicked: {
                var newRight = videoPlot.xBottomAxisRange.x + (videoPlot.xBottomAxisRange.y - videoPlot.xBottomAxisRange.x) * scrollLayout.zoomFactor;
                var rangeCount = newRight - videoPlot.xBottomAxisRange.x + 1

                var originalPosition = videoPlot.xBottomAxisRange.x + (videoPlot.xBottomAxisRange.y - videoPlot.xBottomAxisRange.x) * scroll.position / (1 - scroll.size)
                var originalScale = scroll.size
                var originalRange = videoPlot.xBottomAxisRange.y - videoPlot.xBottomAxisRange.x + 1

                // console.debug('originalPosition: ', originalPosition, 'scroll.position: ', scroll.position, 'scroll.size: ', scroll.size)

                scroll.size = rangeCount / dataModel.total;

                var newLeft = videoPlot.xBottomAxisRange.x;
                var needPositionChange = false;

                if(newRight > (dataModel.total - 1))
                {
                    newLeft = Math.max(0, newLeft - (newRight - (dataModel.total - 1)));
                    newRight = (dataModel.total - 1)

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
            Layout.minimumWidth: 50
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
                var newPos = Math.max(0, x1 / dataModel.total);

                if(newPos < 0)
                    newPos = 0;
                if(newPos > (1 - size))
                    newPos = 1 - size;

                scroll.position = newPos
            }

            function setCustomZoom(x1, x2) {
                var rangeCount = x2 - x1 + 1
                scroll.size = rangeCount / dataModel.total

                videoPlot.xBottomAxisRange = Qt.vector2d(x1, x2)
                audioPlot.xBottomAxisRange = videoPlot.xBottomAxisRange

                scroll.position = Math.max(0, x1 / dataModel.total)
            }

            stepSize: size / 100

            onPositionChanged: {
                console.debug('position changed: ', position)

                var rangeCount = Math.round(videoPlot.xBottomAxisRange.y) - Math.round(videoPlot.xBottomAxisRange.x) + 1
                var from = Math.round(position * dataModel.total);
                var to = Math.round(Math.min(dataModel.total, from + rangeCount)) - 1

                // console.debug('from: ', from, 'to: ', to, 'rangeCount: ', rangeCount, 'to - from: ', to - from)

                videoPlot.xBottomAxisRange = Qt.vector2d(Math.round(to - rangeCount + 1), Math.round(to));
                // console.debug('new rangeCount: ', Math.round(videoPlot.xBottomAxisRange.y) - Math.round(videoPlot.xBottomAxisRange.x),
                // 'from: ', videoPlot.xBottomAxisRange.x, 'to: ', videoPlot.xBottomAxisRange.y)

                audioPlot.xBottomAxisRange = Qt.vector2d(Math.round(to - rangeCount + 1), Math.round(to));
            }
        }

        Text {
            Layout.minimumWidth: 50
            text: videoPlot.xBottomAxisRange.y
        }
    }

}
