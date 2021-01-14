import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import Qt.labs.settings 1.0
import Launcher 0.1
import FileUtils 1.0
import GraphModel 1.0
import QwtQuick2 1.0
import QtQuick.Controls 1.4 as QQC1

Rectangle {
    property alias xmlPath: xmlPath.text

    GraphModel {
        id: graphModel

        signal plotsReady();

        onTotalChanged: {
        }

        onPopulated: {
            console.debug('stopping timer')
            refreshTimer.stop();
            graphModel.update(videoCurve, videoCurve2, audioCurve, audioCurve2);
            zoomAll.clicked();

            plotsReady();
        }

        Component.onCompleted: {
            graphModel.update(videoCurve, videoCurve2, audioCurve, audioCurve2);
        }
    }

    DropArea {
        id: dropArea;
        anchors.fill: parent
        onEntered: {
            drag.accept (Qt.LinkAction);
        }
        onDropped: {
            if(drop.urls.length !== 0)
            {
                var url = drop.urls[0];
                xmlPath.text = FileUtils.getFilePath(url);
                loadButton.clicked();
            }

            console.log(drop.urls)
        }
        onExited: {
        }
    }

    RowLayout {
        id: toolsLayout
        anchors.left: parent.left
        anchors.leftMargin: 20
        anchors.right: parent.right
        anchors.rightMargin: 20

        TextField {
            id: xmlPath
            Layout.fillWidth: true
            selectByMouse: true
        }

        Button {
            text: "select file"
            onClicked: {
                selectPathDialog.callback = (fileUrl) => {
                    var filePath = FileUtils.getFilePath(fileUrl);
                    xmlPath.text = filePath;
                }
                selectPathDialog.open();
            }
        }

        Button {
            text: "load"
            id: loadButton
            onClicked: {
                refreshTimer.start();
                graphModel.reset(videoCurve, videoCurve2, audioCurve, audioCurve2);
                graphModel.populate(xmlPath.text);
            }
        }

        Text {
            text: "Total frames: " + graphModel.total
        }
    }

    QQC1.SplitView {
        id: plotsView

        orientation: Qt.Vertical
        anchors.top: toolsLayout.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: scrollLayout.top

        onHeightChanged: {
            videoLayout.height = plotsView.height / 5 * 3.5
        }

        RowLayout {
            id: videoLayout

            QwtQuick2Plot {
                id: videoPlot
                Layout.fillWidth: true
                Layout.fillHeight: true

                canvasItem.clip: true
                xBottomAxisTitle: "frames, N"
                xBottomAxisEnabled: false
                yLeftAxisTitle: "video error concealment (%)"

                Component.onCompleted: {
                    yLeftAxisFont.bold = false
                    yLeftAxisFont.pixelSize = yLeftAxisFont.pixelSize - 2
                    xBottomAxisFont.bold = false
                    xBottomAxisFont.pixelSize = xBottomAxisFont.pixelSize - 2
                }

                PlotPicker {
                    visible: graphModel.total !== 0
                    overlayTextFormatter: function(p) {
                        return graphModel.videoInfo(p.x, p.y);
                    }
                    onZoomed: scroll.setCustomZoom(x1, x2)
                    onMoved: scroll.move(x1)
                }

                QwtQuick2PlotCurve {
                    id: videoCurve
                    title: "even";
                    curveStyle: QwtQuick2PlotCurve.Sticks
                    color: "darkgreen"
                }

                QwtQuick2PlotCurve {
                    id: videoCurve2
                    title: "odd";
                    curveStyle: QwtQuick2PlotCurve.Sticks
                    color: "green"
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
                yLeftAxisTitle: "audio error (%)"

                Component.onCompleted: {
                    yLeftAxisFont.bold = false
                    yLeftAxisFont.pixelSize = yLeftAxisFont.pixelSize - 2
                    xBottomAxisFont.bold = false
                    xBottomAxisFont.pixelSize = xBottomAxisFont.pixelSize - 2
                }

                PlotPicker {
                    visible: graphModel.total !== 0
                    overlayTextFormatter: function(p) {
                        return graphModel.audioInfo(p.x, p.y);
                    }
                    onZoomed: scroll.setCustomZoom(x1, x2)
                    onMoved: scroll.move(x1)
                }

                QwtQuick2PlotCurve {
                    id: audioCurve
                    title: "even";
                    curveStyle: QwtQuick2PlotCurve.Sticks
                    color: "darkblue"
                }

                QwtQuick2PlotCurve {
                    id: audioCurve2
                    title: "odd"
                    curveStyle: QwtQuick2PlotCurve.Sticks
                    color: "blue"
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
        height: zoomIn.height
        property int zoomFactor: 2

        Button {
            id: zoomIn
            text: "+"
            onClicked: {
                var newRight = videoPlot.xBottomAxisRange.x + (videoPlot.xBottomAxisRange.y - videoPlot.xBottomAxisRange.x) / scrollLayout.zoomFactor;
                var rangeCount = newRight - videoPlot.xBottomAxisRange.x + 1
                scroll.size = rangeCount / graphModel.total

                videoPlot.xBottomAxisRange = Qt.vector2d(videoPlot.xBottomAxisRange.x, newRight)
                audioPlot.xBottomAxisRange = videoPlot.xBottomAxisRange
            }
        }
        Button {
            id: zoomAll
            text: "|"
            onClicked: {
                scroll.size = 1
                scroll.position = 0
                videoPlot.xBottomAxisRange = Qt.vector2d(0, graphModel.total - 1)
                audioPlot.xBottomAxisRange = videoPlot.xBottomAxisRange
            }
        }

        Button {
            id: custom
            text: "Custom"

            Connections {
                target: graphModel
                onPlotsReady: {
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
            id: zoomOut
            text: "-"
            onClicked: {
                var newRight = videoPlot.xBottomAxisRange.x + (videoPlot.xBottomAxisRange.y - videoPlot.xBottomAxisRange.x) * scrollLayout.zoomFactor;
                var rangeCount = newRight - videoPlot.xBottomAxisRange.x + 1

                var originalPosition = videoPlot.xBottomAxisRange.x + (videoPlot.xBottomAxisRange.y - videoPlot.xBottomAxisRange.x) * scroll.position / (1 - scroll.size)
                var originalScale = scroll.size
                var originalRange = videoPlot.xBottomAxisRange.y - videoPlot.xBottomAxisRange.x + 1

                // console.debug('originalPosition: ', originalPosition, 'scroll.position: ', scroll.position, 'scroll.size: ', scroll.size)

                scroll.size = rangeCount / graphModel.total;

                var newLeft = videoPlot.xBottomAxisRange.x;
                var needPositionChange = false;

                if(newRight > (graphModel.total - 1))
                {
                    newLeft = Math.max(0, newLeft - (newRight - (graphModel.total - 1)));
                    newRight = (graphModel.total - 1)

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
                var newPos = Math.max(0, x1 / graphModel.total);

                if(newPos < 0)
                    newPos = 0;
                if(newPos > (1 - size))
                    newPos = 1 - size;

                scroll.position = newPos
            }

            function setCustomZoom(x1, x2) {
                var rangeCount = x2 - x1 + 1
                scroll.size = rangeCount / graphModel.total

                videoPlot.xBottomAxisRange = Qt.vector2d(x1, x2)
                audioPlot.xBottomAxisRange = videoPlot.xBottomAxisRange

                scroll.position = Math.max(0, x1 / graphModel.total)
            }

            stepSize: size / 100

            onPositionChanged: {
                console.debug('position changed: ', position)

                var rangeCount = Math.round(videoPlot.xBottomAxisRange.y) - Math.round(videoPlot.xBottomAxisRange.x) + 1
                var from = Math.round(position * graphModel.total);
                var to = Math.round(Math.min(graphModel.total, from + rangeCount)) - 1

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

    Timer {
        id: refreshTimer
        interval: 500
        running: false
        onTriggered: {
            console.debug('updating plots...')
            graphModel.update(videoCurve, videoCurve2, audioCurve, audioCurve2);
        }
    }
}
