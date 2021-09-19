import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 1.4 as QQC1
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import Qt.labs.settings 1.0
import Launcher 0.1
import FileUtils 1.0
import DataModel 1.0
import QtQuick.Dialogs 1.3
import QtAVPlayerUtils 1.0

Item {
    id: root

    MessageDialog {
        id: errorDialog
        icon: StandardIcon.Critical
        title: "Error on parsing xml"
    }

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
        visible: running
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
        onError: {
            errorDialog.text = "error message: " + errorString
            errorDialog.open()
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
                                      var entries = FileUtils.ls(FileUtils.getFileDir(filePath));
                                      fileViewer.fileView.add(filePath)
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

    SelectPathDialog {
        id: selectPath
        selectMultiple: true
        nameFilters: [
            "Report and video files (*.dvrescue.xml *.mov *.mkv *.avi *.dv *.mxf)",
            "Report files (*.dvrescue.xml)",
            "Video files (*.mov *.mkv *.avi *.dv *.mxf)"
        ]
    }

    RecentsPopup {
        id: recentsPopup
        files: recentFiles
        onSelected: {
            fileViewer.fileView.add(filePath)
        }
    }

    QQC1.SplitView {
        id: splitView
        anchors.fill: parent

        orientation: Qt.Vertical

        onHeightChanged: {
            playerViewSplitView.height = height / 5 * 1.5
        }

        onWidthChanged: {
            dataView.width = width / 2
        }

        QQC1.SplitView {
            id: playerViewSplitView

            orientation: Qt.Horizontal

            onWidthChanged: {
                playerView.width = width / 2
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

                    console.debug('player.onPositionChanged: ', displayPosition)
                    prevDisplayPosition = displayPosition;

                    var ms = displayPosition
                    var frameIndex = ms * fps / 1000;

                    console.debug('player.displayPosition: ', displayPosition, 'frameIndex: ', frameIndex)
                    playerView.positionChanged(Math.round(frameIndex));
                }

                function seekToFrame(frameIndex) {
                    var position = frameIndex / playerView.fps * 1000
                    playerView.player.seek(position);
                }

                Connections {
                    target: dataView
                    onTapped: {
                        if(framePos !== -1)
                            playerView.seekToFrame(framePos);
                    }
                }
            }

            ColumnLayout {
                id: fileViewColumn
                anchors.top: parent.top
                anchors.bottom: parent.bottom

                spacing: 0

                AnalyseFileViewer {
                    id: fileViewer
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    fileView.currentIndex: fileSelector.currentIndex

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

                    CustomButton {
                        icon.source: "icons/add-files.svg"
                        onClicked: {
                            selectPath.callback = (urls) => {
                                urls.forEach((url) => {
                                                 fileViewer.fileView.add(FileUtils.getFilePath(url));
                                             });
                            }

                            selectPath.open();
                        }
                    }

                    CustomButton {
                        icon.source: "icons/recent.svg"

                        onClicked: {
                            var mapped = mapToItem(root, 0, 0);
                            recentsPopup.x = mapped.x - recentsPopup.width + width
                            recentsPopup.y = mapped.y + height

                            recentsPopup.open();
                        }
                    }

                    CustomButton {
                        icon.source: "icons/recent.svg"

                        onClicked: {
                            var path = fileViewer.fileView.fileInfos[fileViewer.fileView.currentIndex].reportPath
                            if(Qt.platform.os === "windows") {
                                path = "/cygdrive/" + path.replace(":", "");
                            }

                            var extraParams = " -v -X {xml} -F {ffmpeg} -D {dvrescue} -M {mediainfo}"
                                .replace("{xml}", packagerCtl.effectiveXmlStarletCmd)
                                .replace("{ffmpeg}", packagerCtl.effectiveFfmpegCmd)
                                .replace("{dvrescue}", packagerCtl.effectiveDvrescueCmd)
                                .replace("{mediainfo}", packagerCtl.effectiveMediaInfoCmd)

                            var output = '';
                            packagerCtl.exec("-T " + path, (launcher) => {
                                debugView.logCommand(launcher)
                                launcher.outputChanged.connect((outputString) => {
                                    output += outputString;
                                })
                            }, extraParams).then(() => {
                                console.debug('executed....')
                                debugView.logResult(output);

                                busy.running = false;
                            }).catch((error) => {
                                debugView.logResult(error);
                                busy.running = false;
                            });
                        }
                    }

                    ComboBox {
                        id: fileSelector
                        model: fileViewer.updated, fileViewer.files
                        Layout.fillWidth: true
                        currentIndex: fileViewer.fileView.currentIndex
                        onCurrentIndexChanged: {
                            if(fileViewer.files.length > currentIndex)
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

                        if(filePath == undefined || filePath.length === 0) {
                            playerView.player.source = '';
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

        }

        QQC1.SplitView {
            id: tableAndPlots
            orientation: Qt.Horizontal

            AnalyseDataView {
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

                onMarkerClicked: {
                    playerView.seekToFrame(frameIndex)
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
