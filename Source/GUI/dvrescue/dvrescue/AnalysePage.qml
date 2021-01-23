import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 1.4 as QQC1
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import Qt.labs.settings 1.0
import Launcher 0.1
import FileUtils 1.0
import DataModel 1.0
import QtAVPlayerUtils 1.0

Item {
    property alias xmlPath: filePathTextField.text

    FileViewer {
        id: fileViewer
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.rightMargin: 10
        anchors.topMargin: 10
    }

    DataModel {
        id: dataModel

        onTotalChanged: {
        }

        onPopulated: {
            console.debug('stopping timer')
            refreshTimer.stop();
            dataModel.update(plotsView.evenVideoCurve, plotsView.oddVideoCurve,
                              plotsView.evenAudioCurve, plotsView.oddAudioCurve);
        }

        Component.onCompleted: {
            dataModel.update(plotsView.evenVideoCurve, plotsView.oddVideoCurve,
                              plotsView.evenAudioCurve, plotsView.oddAudioCurve);
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
                var filePath = FileUtils.getFilePath(url);

                filePath.text = filePath;
                loadButton.clicked();
            }

            console.log(drop.urls)
        }
        onExited: {
        }
    }

    RowLayout {
        id: toolsLayout
        anchors.top: fileViewer.bottom
        anchors.topMargin: 5
        anchors.left: parent.left
        anchors.leftMargin: 0
        anchors.right: parent.right
        anchors.rightMargin: 0

        Rectangle  {
            id: textFieldRect
            Layout.fillWidth: true
            height: childrenRect.height

            TextField {
                id: filePathTextField
                selectByMouse: true
                width: parent.width
            }
        }

        Button {
            text: "select file"
            onClicked: {
                selectPathDialog.callback = (fileUrl) => {
                    var path = FileUtils.getFilePath(fileUrl);
                    filePathTextField.text = path;
                }
                selectPathDialog.open();
            }
        }

        Button {
            text: "load"
            id: loadButton

            property string dvRescueXmlExtension: ".dvrescue.xml"

            onClicked: {

                dataModel.reset(plotsView.evenVideoCurve, plotsView.oddVideoCurve,
                                 plotsView.evenAudioCurve, plotsView.oddAudioCurve);

                var extension = FileUtils.getFileExtension(filePathTextField.text);
                if(extension === 'xml') {
                    refreshTimer.start();
                    dataModel.populate(filePathTextField.text);

                    if(filePathTextField.text.endsWith(dvRescueXmlExtension))
                    {
                        var videoPath = filePathTextField.text.substring(0, filePathTextField.text.length - dvRescueXmlExtension.length);
                        if(FileUtils.exists(videoPath))
                        {
                            playerView.player.source = 'file:///' + videoPath;
                            playerView.player.playPaused(0);
                        }
                    }

                } else {
                    var dvRescueXmlPath = filePathTextField.text + dvRescueXmlExtension
                    if(FileUtils.exists(dvRescueXmlPath))
                    {
                        refreshTimer.start();
                        dataModel.populate(dvRescueXmlPath);
                    }

                    playerView.player.source = 'file:///' + filePathTextField.text;
                    playerView.player.playPaused(0);
                }
            }
        }

        Rectangle  {
            width: 200
            height: textFieldRect.height
            Text {
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                id: framesInfo
                text: "Total frames: " + dataModel.total + ", fps: " + playerView.fps

            }
        }
    }

    QQC1.SplitView {
        id: splitView
        anchors.top: toolsLayout.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        orientation: Qt.Vertical

        onHeightChanged: {
            playerView.height = height / 5 * 1.5
        }

        onWidthChanged: {
            dataView.width = width / 2
        }

        PlayerView {
            id: playerView
            signal positionChanged(int frameIndex);

            onFpsChanged: {
                if(fps !== 0) {
                    plotsView.framePos = 0
                } else {
                    plotsView.framePos = -1
                }
            }

            property var prevDisplayPosition: -1
            player.onPositionChanged: {
                var displayPosition = QtAVPlayerUtils.displayPosition(player)
                if(prevDisplayPosition === displayPosition)
                    return;

                prevDisplayPosition = displayPosition;

                var ms = displayPosition
                var frameIndex = ms * fps / 1000;

                console.debug('player.displayPosition: ', displayPosition, 'frameIndex: ', frameIndex)
                positionChanged(frameIndex);
            }
        }

        QQC1.SplitView {
            id: tableAndPlots
            orientation: Qt.Horizontal

            DataView {
                id: dataView

                Component.onCompleted: {
                    dataModel.bind(model)
                }
            }

            PlotsView {
                id: plotsView

                Connections {
                    target: dataModel
                    function onPopulated() {
                        plotsView.zoomAll();
                    }
                }

                Connections {
                    target: playerView
                    function onPositionChanged(frameIndex) {
                        plotsView.framePos = frameIndex
                    }
                }

                onPickerMoved: {
                    var frameIndex = plotX
                    var position = frameIndex / playerView.fps * 1000

                    playerView.player.seek(position);
                }
            }

        }
    }

    Timer {
        id: refreshTimer
        interval: 500
        running: false
        onTriggered: {
            console.debug('updating plots...')
            dataModel.update(plotsView.evenVideoCurve, plotsView.oddVideoCurve,
                              plotsView.evenAudioCurve, plotsView.oddAudioCurve);
        }
    }
}
