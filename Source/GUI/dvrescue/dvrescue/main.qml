import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import Qt.labs.settings 1.0
import Launcher 0.1
import FileUtils 1.0
import GraphModel 1.0
import QwtQuick2 1.0

Window {
    id: root
    width: 1280
    height: 720
    visible: true
    title: qsTr("Hello World")

    GraphModel {
        id: graphModel

        onTotalChanged: {
            console.debug('total: ', total)
        }

        onPopulated: {
            console.debug('stopping timer')
            refreshTimer.stop();
            graphModel.update(videoCurve, videoCurve2, audioCurve, audioCurve2);
            zoomAll.clicked();
        }
    }

    Rectangle {
        anchors.fill: parent;
        z: 100

        RowLayout {
            id: toolsLayout
            anchors.left: parent.left
            anchors.leftMargin: 20
            anchors.right: parent.right
            anchors.rightMargin: 20

            TextField {
                id: xmlPath
                Layout.fillWidth: true
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
                onClicked: {
                    refreshTimer.start();
                    graphModel.populate(xmlPath.text);
                }
            }

            Text {
                text: "Total frames: " + graphModel.total
            }
        }

        QwtQuick2Plot {
            id: videoPlot
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.topMargin: 10
            anchors.top: toolsLayout.bottom
            canvasItem.clip: true
            height: (parent.height - toolsLayout.height - scrollLayout.height) / 2 - anchors.topMargin
            xBottomAxisTitle: "frames, N"
            yLeftAxisTitle: "video error concealment (%)"

            PlotPicker {
            }

            QwtQuick2PlotCurve {
                id: videoCurve
                title: "Video errors (even)";
                curveStyle: QwtQuick2PlotCurve.Sticks
                color: "darkgreen"
            }

            QwtQuick2PlotCurve {
                id: videoCurve2
                title: "Video errors (odd)";
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

        QwtQuick2Plot {
            id: audioPlot
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.topMargin: 10
            anchors.top: videoPlot.bottom
            canvasItem.clip: true
            height: videoPlot.height
            xBottomAxisTitle: "frames, N"
            yLeftAxisTitle: "audio error (%)"

            PlotPicker {
            }

            QwtQuick2PlotCurve {
                id: audioCurve
                title: "Audio errors (even)";
                curveStyle: QwtQuick2PlotCurve.Sticks
                color: "darkblue"
            }

            QwtQuick2PlotCurve {
                id: audioCurve2
                title: "Audio errors (odd)"
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

        RowLayout {
            id: scrollLayout
            anchors.top: audioPlot.bottom
            anchors.left: audioPlot.left
            anchors.right: audioPlot.right
            height: zoomIn.height
            property int zoomFactor: 2

            Button {
                id: zoomIn
                text: "+"
                onClicked: {
                    videoPlot.xBottomAxisRange = Qt.vector2d(videoPlot.xBottomAxisRange.x, Math.round(videoPlot.xBottomAxisRange.y / scrollLayout.zoomFactor))
                    audioPlot.xBottomAxisRange = videoPlot.xBottomAxisRange
                }
            }
            Button {
                id: zoomAll
                text: "|"
                onClicked: {
                    scroll.position = 0
                    videoPlot.xBottomAxisRange = Qt.vector2d(0, graphModel.total)
                    audioPlot.xBottomAxisRange = videoPlot.xBottomAxisRange
                }
            }

            Button {
                id: zoomOut
                text: "-"
                onClicked: {
                    var newRange = Qt.vector2d(videoPlot.xBottomAxisRange.x, videoPlot.xBottomAxisRange.y * scrollLayout.zoomFactor);
                    if((newRange.y - newRange.x) > graphModel.total)
                    {
                        zoomAll.clicked();
                    } else {
                        videoPlot.xBottomAxisRange = newRange
                        audioPlot.xBottomAxisRange = videoPlot.xBottomAxisRange
                    }
                }
            }

            ScrollBar {
                id: scroll
                orientation: "Horizontal"
                height: parent.height
                hoverEnabled: true
                active: true
                policy: ScrollBar.AlwaysOn
                Layout.fillWidth: true
                size: {
                    if(graphModel.total == 0)
                        return 0;

                    var rangeCount = Math.round(videoPlot.xBottomAxisRange.y) - Math.round(videoPlot.xBottomAxisRange.x) + 1
                    var value = rangeCount / graphModel.total

                    console.debug('size: ', value, videoPlot.xBottomAxisRange.y, videoPlot.xBottomAxisRange.x)

                    return value;
                }
                onPositionChanged: {
                    // console.debug('position changed: ', position)

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

    AvfCtl {
        id: avfctl
    }

    Settings {
        id: settings;
        property alias avfctlCmd: avfctl.avfctlCmd
        property alias selectedXml: xmlPath.text
    }

    Dialog {
        id: avfctlDialog
        title: "Please, specify avfctl tool location"
        contentWidth: 480

        TextField {
            id: avfctlField
            width: 480

            placeholderText: "avfctl tool path..."
            selectByMouse: true
        }

        standardButtons: Dialog.Cancel | Dialog.Ok
        onAccepted: {
            avfctl.avfctlCmd = avfctlField.text
        }
        anchors.centerIn: parent
    }

    SelectPathDialog {
        id: selectPathDialog
    }

    SpecifyPathDialog {
        id: specifyPathDialog
    }

    SimpleTransfer {
        width: 1280
        height: 720

        property var urlToPath: function(url) {
            return FileUtils.getFilePath(url);
        }

        rewindMouseArea.onClicked: {
            avfctl.rew(0, (launcher) => {
               commandsLogs.logCommand(launcher);
            }).then((result) => {
               commandsLogs.logResult(result.outputText);
               return result;
            });
        }

        stopMouseArea.onClicked: {
            avfctl.stop(0, (launcher) => {
               commandsLogs.logCommand(launcher);
            }).then((result) => {
               commandsLogs.logResult(result.outputText);
               return result;
            });
        }

        playMouseArea.onClicked: {
            avfctl.play(0, (launcher) => {
               commandsLogs.logCommand(launcher);
            }).then((result) => {
               commandsLogs.logResult(result.outputText);
               return result;
            });
        }

        fastForwardMouseArea.onClicked: {
            avfctl.ff(0,  (launcher) => {
                commandsLogs.logCommand(launcher);
            }).then((result) => {
                commandsLogs.logResult(result.outputText);
                return result;
            });
        }

        grabMouseArea.onClicked: {
            specifyPathDialog.callback = (fileUrl) => {
                var filePath = urlToPath(fileUrl);

                avfctl.grab(0, filePath, (launcher) => {
                   console.debug('logging grab command')
                   commandsLogs.logCommand(launcher);
                }).then((result) => {
                   commandsLogs.logResult(result.outputText);
                   return result;
                }).catch((e) => {
                   commandsLogs.logResult(e);
                });
            }

            specifyPathDialog.open();
        }

        settingsMouseArea.onClicked: {
            avfctlField.text = avfctl.avfctlCmd
            avfctlField.forceActiveFocus();
            avfctlDialog.open();
        }

        deviceNameTextField.text: devicesModel.count === 0 ? '' : devicesModel.get(0).name + " (" + devicesModel.get(0).type + ")"
    }

    DevicesModel {
        id: devicesModel
    }

    DevicesModelUpdater {
        model: devicesModel
        getDevices: function() {
            return avfctl.queryDecks((launcher) => {
                statusLogs.logCommand(launcher);
            }).then((result) => {
                statusLogs.logResult(result.outputText);
                return result;
            });
        }
    }

    Rectangle {
        id: debugRect
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        width: 162
        height: 100
        color: "#a40000"

        Text {
            anchors.centerIn: parent
            color: "#ffffff"
            text: qsTr("Debug")
            font.pixelSize: 26
            font.family: "Tahoma"
            minimumPixelSize: 14
            font.weight: Font.Bold
            minimumPointSize: 14
        }
    }

    MouseArea {
        anchors.fill: debugRect
        onClicked: {
            debugView.visible = !debugView.visible
        }
    }

    Window {
        id: debugView
        visible: false
        width: root.width / 2
        height: root.height

        Component.onCompleted: {
            x = root.width
            y = root.y
        }

        Rectangle {
            width: parent.width / 2
            color: "#ccffffff"

            TabBar {
                id: tabBar
                width: parent.width
                currentIndex: 0

                TabButton {
                    text: "command logs"
                }
                TabButton {
                    text: "status logs"
                }
            }

            StackLayout {
                anchors.top: tabBar.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                currentIndex: tabBar.currentIndex

                ScrollView {
                    TextArea {
                        id: commandsLogs
                        selectByMouse: true

                        function logCommand(launcher) {
                            console.debug('logging command: ', launcher.program() + ' ' + launcher.arguments().join(' '))

                            append(launcher.program() + ' ' + launcher.arguments().join(' '));
                            append('\n');
                        }

                        function logResult(result) {
                            append(result);
                            append('\n\n');
                        }

                        Component.onCompleted: {
                            append("beginning commands log");
                        }
                    }
                }

                ScrollView {
                    TextArea {
                        id: statusLogs
                        selectByMouse: true

                        function logCommand(launcher) {
                            if(text.length > 50000)
                                clear();

                            append(launcher.program() + ' ' + launcher.arguments().join(' '));
                            append('\n');
                        }

                        function logResult(result) {
                            append(result);
                            append('\n\n');
                        }

                        Component.onCompleted: {
                            append("beginning status log");
                        }
                    }
                }
            }
        }
    }
}
