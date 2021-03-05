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

    Rectangle {
        z: 100
        color: 'darkgray'
        opacity: 0.5
        anchors.fill: parent
        visible: busy.running

        MouseArea {
            anchors.fill: parent
        }
    }

    BusyIndicator {
        id: busy
        z: 100
        width: Math.min(parent.width, parent.height) / 3
        height: width;
        running: false
        anchors.centerIn: parent
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
                drop.urls.forEach((url) => {
                                      var filePath = FileUtils.getFilePath(url);
                                      fileViewer.fileView.add(filePath)
                                      addRecent(filePath)
                                  })
            }

            console.log(drop.urls)
        }
        onExited: {
        }
    }

    property string recentFilesJSON : ''
    property var recentFiles: recentFilesJSON === '' ? [] : JSON.parse(recentFilesJSON)
    function addRecent(filePath) {
        var newRecentFiles = recentFiles.filter(item => item !== filePath)
        newRecentFiles.unshift(filePath)

        if(newRecentFiles.length > 10) {
            newRecentFiles.pop();
        }
        recentFiles = newRecentFiles
        recentFilesJSON = JSON.stringify(recentFiles)
    }

    QQC1.SplitView {
        anchors.fill: parent

        orientation: Qt.Vertical

        onHeightChanged: {
            fileViewColumn.height = height / 5 * 1
            fileViewer.fileView.tableView.bringToView(0);
        }

        ColumnLayout {
            id: fileViewColumn
            anchors.left: parent.left
            anchors.right: parent.right

            spacing: 0

            FileViewer {
                id: fileViewer
                Layout.fillWidth: true
                Layout.fillHeight: true
                fileView.currentIndex: fileSelector.currentIndex
                recentsPopup.files: recentFiles
                recentsPopup.onSelected: {
                    fileViewer.fileView.add(filePath)
                }

                function resolveMediaInfo(index, reportPath) {
                    var mediaInfo = fileView.mediaInfoAt(index)
                    mediaInfo.reportPath = reportPath;
                    mediaInfo.resolve();
                }

                fileView.onFileAdded: {
                    addRecent(filePath)
                }

                onSelectedPathChanged: {
                    console.debug('selected path: ', selectedPath)
                    toolsLayout.load(selectedPath, fileView.currentIndex)
                }
            }

            RowLayout {
                id: toolsLayout
                Layout.fillWidth: true

                property string dvRescueXmlExtension: ".dvrescue.xml"
                property int fileViewerHeight: 0

                Button {
                    visible: !fileViewer.visible
                    text: "^"
                    onClicked: {
                        fileViewer.visible = true
                        fileViewColumn.height = toolsLayout.fileViewerHeight
                    }
                }

                Button {
                    visible: fileViewer.visible
                    text: ">"
                    onClicked: {
                        toolsLayout.fileViewerHeight = fileViewColumn.height
                        fileViewer.visible = false
                    }
                }

                ComboBox {
                    id: fileSelector
                    model: fileViewer.updated, fileViewer.files
                    Layout.fillWidth: true
                    currentIndex: fileViewer.fileView.currentIndex
                    onCurrentIndexChanged: {
                        if(fileViewer.files.length < currentIndex)
                        {
                            var file = fileViewer.files[currentIndex]
                            toolsLayout.load(file, currentIndex)
                        }
                    }
                }

                function load(filePath, currentIndex) {

                    console.debug('load: ', filePath, 'currentIndex: ', currentIndex)

                    dataModel.reset(plotsView.evenVideoCurve, plotsView.oddVideoCurve,
                                     plotsView.evenAudioCurve, plotsView.oddAudioCurve);

                    if(filePath.length === 0) {
                        playerView.player.stop()
                        return;
                    }

                    var extension = FileUtils.getFileExtension(filePath);
                    if(extension === 'xml') {
                        refreshTimer.start();
                        dataModel.populate(filePath);

                        if(filePath.endsWith(dvRescueXmlExtension))
                        {
                            var videoPath = filePath.substring(0, filePath.length - dvRescueXmlExtension.length);
                            if(FileUtils.exists(videoPath))
                            {
                                playerView.player.source = 'file:///' + videoPath;
                                playerView.player.playPaused(0);
                            }
                        }

                    } else {
                        var dvRescueXmlPath = filePath + dvRescueXmlExtension
                        if(FileUtils.exists(dvRescueXmlPath))
                        {
                            refreshTimer.start();
                            dataModel.populate(dvRescueXmlPath);
                        } else {
                            busy.running = true;
                            dvrescue.makeReport(filePath).then(() => {
                                 busy.running = false;
                                 refreshTimer.start();
                                 dataModel.populate(dvRescueXmlPath);

                                 if(currentIndex !== -1 && currentIndex !== undefined) {
                                    fileViewer.resolveMediaInfo(currentIndex, dvRescueXmlPath)
                                 }
                            }).catch((error) => {
                                busy.running = false;
                            });
                        }

                        playerView.player.source = 'file:///' + filePath;
                        playerView.player.playPaused(0);
                    }
                }
            }
        }

        QQC1.SplitView {
            id: splitView
            anchors.left: parent.left
            anchors.right: parent.right

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
                    playerView.positionChanged(Math.round(frameIndex));
                }

                function seekToFrame(frameIndex) {
                    var position = frameIndex / playerView.fps * 1000
                    playerView.player.seekEx(position);
                }

                Connections {
                    target: dataView
                    onTapped: {
                        if(framePos !== -1)
                            playerView.seekToFrame(framePos);
                    }
                }
            }

            QQC1.SplitView {
                id: tableAndPlots
                orientation: Qt.Horizontal

                DataView {
                    id: dataView
                    cppDataModel: dataModel

                    Connections {
                        target: playerView
                        onPositionChanged: {
                            dataView.framePos = frameIndex
                            dataView.bringToView(dataView.framePos)
                        }
                    }

                    Component.onCompleted: {
                        dataModel.bind(model)
                    }
                }

                PlotsView {
                    id: plotsView

                    Connections {
                        target: dataModel
                        onPopulated: {
                            plotsView.zoomAll();
                        }
                    }

                    Connections {
                        target: playerView
                        onPositionChanged: {
                            plotsView.framePos = frameIndex
                        }
                    }

                    onPickerMoved: {
                        var frameIndex = plotX
                        playerView.seekToFrame(frameIndex)
                    }
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
