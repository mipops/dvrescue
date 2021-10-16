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
    property int startFrame: 0
    property int endFrame: dataModel.total - 1

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
        orientation: Qt.Horizontal

        onWidthChanged: {
            playerAndPlotsSplitView.width = width / 2
        }

        QQC1.SplitView {
            id: playerAndPlotsSplitView

            orientation: Qt.Vertical

            onHeightChanged: {
                playerView.height = height / 2
            }

            PlayerView {
                id: playerView

                startOffset: fps == 0 ? 0 : (root.startFrame / fps * 1000)
                endOffset: fps == 0 ? player.duration : (root.endFrame / fps * 1000)

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

            PlotsView {
                id: plotsView
                startFrame: root.startFrame
                endFrame: root.endFrame
                overlay: segmentDataView.hoveredItem != null && segmentDataView.currentIndex === -1 ? segmentDataView.hoveredItem.range : Qt.vector2d(-1, -1)

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

        QQC1.SplitView {
            id: tables
            orientation: Qt.Vertical

            onHeightChanged: {
                fileViewColumn.height = height / 5 * 1.5
            }

            ColumnLayout {
                id: fileViewColumn
                spacing: 0

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

                    /*
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
                    */

                    ComboBox {
                        id: fileSelector
                        textRole: "fileName"
                        model: fileViewer.updated, fileViewer.fileInfos
                        Layout.fillWidth: true

                        currentIndex: fileViewer.fileView.currentIndex
                        onActivated: {
                            fileViewer.fileView.currentIndex = index
                        }
                    }

                    TabBar {
                        Layout.minimumWidth: 250
                        id: fileSegmentSwitch
                        currentIndex: 0

                        TabButton {
                            text: "file"
                        }
                        TabButton {
                            text: "segment"
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

                                segmentDataView.populateSegmentData(dvRescueXmlPath)
                            } else {
                                busy.running = true;
                                dvrescue.makeReport(filePath).then(() => {
                                                                       busy.running = false;
                                                                       refreshTimer.start();
                                                                       dataModel.populate(dvRescueXmlPath);

                                                                       if(currentIndex !== -1 && currentIndex !== undefined) {
                                                                           fileViewer.fileView.fileInfos[currentIndex].reportPath = dvRescueXmlPath;

                                                                           var mediaInfo = fileViewer.fileView.mediaInfoAt(currentIndex)
                                                                           mediaInfo.reportPath = dvRescueXmlPath;
                                                                           mediaInfo.resolve();

                                                                           segmentDataView.populateSegmentData(mediaInfo.reportPath)
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

                StackLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    currentIndex: fileSegmentSwitch.currentIndex

                    AnalyseFileViewer {
                        id: fileViewer
                        fileView.currentIndex: fileSelector.currentIndex

                        fileView.onFileAdded: {
                            addRecent(filePath)
                        }

                        onSelectedPathChanged: {
                            console.debug('selected path: ', selectedPath)
                            toolsLayout.load(selectedPath, fileView.currentIndex)
                        }
                    }

                    ColumnLayout {
                        Rectangle {
                            color: 'white'
                            Layout.fillWidth: true
                            Layout.minimumHeight: childrenRect.height

                            Flow {
                                id: segmentationOptionsLayout
                                width: parent.width
                                property bool needsApply: false;

                                Label {
                                    text: "Segmenting Rules"
                                    font.bold: true
                                    verticalAlignment: Text.AlignVCenter
                                    height: recordingStartMarkers.implicitHeight
                                }

                                CheckBox {
                                    id: recordingStartMarkers
                                    text: "Rec Start Markers"
                                    onCheckedChanged: {
                                        segmentationOptionsLayout.needsApply = true
                                    }
                                }
                                CheckBox {
                                    id: breaksInRecordingTime
                                    text: "Rec Time Break"
                                    onCheckedChanged: {
                                        segmentationOptionsLayout.needsApply = true
                                    }
                                }
                                CheckBox {
                                    id: breaksInTimecode
                                    text: "Timecode Break"
                                    onCheckedChanged: {
                                        segmentationOptionsLayout.needsApply = true
                                    }
                                }
                                CheckBox {
                                    id: segmentFilesToPreserveAudioSampleRate
                                    text: "Audio Setting Change"
                                    onCheckedChanged: {
                                        segmentationOptionsLayout.needsApply = true
                                    }
                                }

                                Label {
                                    text: "Aspect Ratio Change"
                                    verticalAlignment: Text.AlignVCenter
                                    height: recordingStartMarkers.implicitHeight
                                    font.bold: true
                                }

                                ComboBoxEx {
                                    id: aspectRatiosSelector
                                    sizeToContents: true
                                    model: [
                                        "Yes, segment frames by aspect ratio changes",
                                        "No and use most common aspect ratio",
                                        "No and force segments to use 4/3",
                                        "No and force segments to use 16/9"
                                    ]
                                    onCurrentIndexChanged: {
                                        segmentationOptionsLayout.needsApply = true
                                    }
                                }

                                Button {
                                    text: "Reset"
                                    onClicked: {
                                        if(recordingStartMarkers.checked) {
                                            recordingStartMarkers.checked = false
                                            segmentationOptionsLayout.needsApply = true
                                        }

                                        if(breaksInRecordingTime.checked) {
                                            breaksInRecordingTime.checked = false
                                            segmentationOptionsLayout.needsApply = true
                                        }

                                        if(breaksInTimecode.checked) {
                                            breaksInTimecode.checked = false
                                            segmentationOptionsLayout.needsApply = true
                                        }

                                        if(segmentFilesToPreserveAudioSampleRate.checked) {
                                            segmentFilesToPreserveAudioSampleRate.checked = false
                                            segmentationOptionsLayout.needsApply = true
                                        }

                                        if(aspectRatiosSelector.currentIndex !== 0) {
                                            aspectRatiosSelector.currentIndex = 0
                                            segmentationOptionsLayout.needsApply = true
                                        }
                                    }
                                }

                                Button {
                                    enabled: segmentationOptionsLayout.needsApply
                                    text: "Apply"
                                    onClicked: {
                                        segmentDataView.repopulateSegmentData()
                                        segmentationOptionsLayout.needsApply = false
                                    }
                                }
                            }
                        }

                        SegmentDataView {
                            id: segmentDataView
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            total: dataModel.total

                            onClicked: {
                                console.debug('clicked: ', index, JSON.stringify(item));
                                if(index !== currentIndex) {
                                    currentIndex = -1;
                                    startFrame = 0;
                                    endFrame = total - 1;

                                    dataView.invalidateFilter();
                                }
                            }

                            onDoubleClicked: {
                                console.debug('doubleclicked: ', index, JSON.stringify(item));
                                if(index !== currentIndex) {
                                    currentIndex = index;
                                    startFrame = item['Frame #']
                                    endFrame = total - 1;
                                    if((currentIndex + 1) < model.rowCount) {
                                        endFrame = model.getRow(currentIndex + 1)['Frame #']
                                    }

                                    dataView.invalidateFilter();
                                }
                            }

                            Component.onCompleted: {
                                var e = {
                                    'Segment #' : '',
                                    'Frame #' : '',
                                    'Timestamp' : '',
                                    'Timecode' : '',
                                    'Timecode: Jump/Repeat' : Qt.point(0, 0),
                                    'Recording Time' : '',
                                    'Recording Time: Jump/Repeat' : Qt.point(0, 0),
                                    'Recording Marks' : Qt.point(0, 0),
                                    'Video/Audio' : ''
                                }

                                segmentDataView.model.appendRow(e);
                                segmentDataView.model.clear();
                            }

                            SegmentDataParser {
                                id: segmentDataParser
                            }

                            function repopulateSegmentData() {
                                var reportPath = fileViewer.fileView.fileInfos[fileViewer.fileView.currentIndex].reportPath
                                segmentDataView.populateSegmentData(reportPath)
                            }

                            function populateSegmentData(reportPath) {
                                segmentDataView.model.clear();

                                var path = reportPath
                                if(Qt.platform.os === "windows") {
                                    path = "/cygdrive/" + path.replace(":", "");
                                }

                                var extraParams = " -v -X {xml} -F {ffmpeg} -D {dvrescue} -M {mediainfo}"
                                    .replace("{xml}", packagerCtl.effectiveXmlStarletCmd)
                                    .replace("{ffmpeg}", packagerCtl.effectiveFfmpegCmd)
                                    .replace("{dvrescue}", packagerCtl.effectiveDvrescueCmd)
                                    .replace("{mediainfo}", packagerCtl.effectiveMediaInfoCmd)

                                var opts = ' ';
                                if(recordingStartMarkers.checked)
                                    opts += '-s '
                                if(breaksInRecordingTime.checked)
                                    opts += '-d ';
                                if(breaksInTimecode.checked)
                                    opts += '-t ';
                                if(segmentFilesToPreserveAudioSampleRate.checked)
                                    opts += '-3 ';

                                if(aspectRatiosSelector.currentIndex === 0)
                                    opts += '-a n ';
                                if(aspectRatiosSelector.currentIndex === 2)
                                    opts += '-a 4 ';
                                if(aspectRatiosSelector.currentIndex === 3)
                                    opts += '-a 9 ';
                                if(aspectRatiosSelector.currentIndex === 1)
                                    opts += '-a c ';

                                var output = '';
                                packagerCtl.exec("-T" + opts + path, (launcher) => {
                                    debugView.logCommand(launcher)
                                    launcher.outputChanged.connect((outputString) => {
                                        output += outputString;
                                    })
                                }, extraParams).then(() => {
                                    console.debug('executed....')
                                    debugView.logResult(output);

                                    var i = 0;
                                    segmentDataParser.parse(output, (entry) => {
                                        console.debug('entry: ', JSON.stringify(entry));

                                        var videoAudio = [
                                            entry.frameSize,
                                            entry.frameRate,
                                            entry.chromaSubsampling,
                                            entry.aspectRatio,
                                            entry.samplingRate,
                                            entry.channelCount
                                        ]

                                        ++i
                                        var e = {
                                            'Segment #' : i,
                                            'Frame #' : entry.startFrame,
                                            'Timestamp' : entry.startPts,
                                            'Timecode' : entry.timeCode,
                                            'Timecode: Jump/Repeat' : Qt.point(entry.timeCodeJump, 0),
                                            'Recording Time' : entry.recTimestamp,
                                            'Recording Time: Jump/Repeat' : Qt.point(entry.recTimeJump, 0),
                                            'Recording Marks' : Qt.point(entry.recStart, 0),
                                            'Video/Audio' : videoAudio.join(' ')
                                        }

                                        segmentDataView.model.appendRow(e);
                                    });

                                    busy.running = false;
                                }).catch((error) => {
                                    debugView.logResult(error);
                                    busy.running = false;
                                });
                            }
                        }
                    }
                }
            }

            AnalyseDataView {
                id: dataView
                cppDataModel: dataModel
                ranges: segmentDataView.hoveredItem && segmentDataView.currentIndex === -1 ? segmentDataView.hoveredItem.range : Qt.vector2d(-1, -1)
                rowFilter: function(index) {
                    var data = dataView.model.getRow(index);
                    var frame = data['Frame #'];
                    if(frame < root.startFrame)
                        return false;

                    if(frame > root.endFrame)
                        return false;

                    return true;
                }

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
