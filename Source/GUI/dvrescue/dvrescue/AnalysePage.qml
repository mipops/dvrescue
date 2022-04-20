import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import Qt.labs.settings 1.0
import Launcher 0.1
import FileUtils 1.0
import ImageUtils 1.0
import LoggingUtils 1.0
import DataModel 1.0
import Dialogs 1.0
import QtAVPlayerUtils 1.0
import SplitView 1.0

Item {
    id: root

    property int startFrame: 0
    onStartFrameChanged: {
        console.debug('analyse page: startFrame = ', startFrame)
    }

    property int endFrame: dataModel.total - 1
    onEndFrameChanged: {
        console.debug('analyse page: endFrame = ', endFrame)
    }

    property alias filesModel: fileView.filesModel
    property alias recentFilesModel: recentsPopup.filesModel
    property int framesCount: dataModel.total
    onFramesCountChanged: {
        console.debug('analyse page: framesCount = ', framesCount)
    }

    MessageDialog {
        id: errorDialog
        // icon: StandardIcon.Critical
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
            console.debug('total changed: ', total)
        }

        onPopulated: {
            console.debug('stopping timer')
            refreshTimer.stop();

            root.startFrame = 0;
            root.endFrame = dataModel.total - 1;
            dataView.invalidateFilter();

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
                                      filesModel.add(filePath)
                                  })
            }

            console.log(drop.urls)
        }
        onExited: {
        }
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
        onSelected: {
            root.filesModel.add(filePath)
        }
    }

    SplitView {
        id: splitView
        anchors.fill: parent
        orientation: Qt.Horizontal

        onWidthChanged: {
            playerAndPlotsSplitView.width = width / 2
        }

        SplitView {
            id: playerAndPlotsSplitView

            orientation: Qt.Vertical

            onHeightChanged: {
                playerView.height = height / 2
            }

            Component.onCompleted: {
                SplitView.preferredWidth = Qt.binding(function() { return playerView.width  })
            }

            PlayerView {
                id: playerView

                Component.onCompleted: {
                    SplitView.preferredHeight = Qt.binding(function() { return height })
                }

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
                overlay: segmentDataViewWithToolbar.hoveredItem != null && segmentDataViewWithToolbar.currentIndex === -1 ? segmentDataViewWithToolbar.hoveredItem.range : Qt.vector2d(-1, -1)

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

        SplitView {
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
                        icon.source: "/icons/add-files.svg"
                        onClicked: {
                            selectPath.callback = (urls) => {
                                urls.forEach((url) => {
                                                 filesModel.add(FileUtils.getFilePath(url));
                                             });
                            }

                            selectPath.open();
                        }
                    }

                    CustomButton {
                        icon.source: "/icons/recent.svg"

                        onClicked: {
                            var mapped = mapToItem(root, 0, 0);
                            recentsPopup.x = mapped.x - recentsPopup.width + width
                            recentsPopup.y = mapped.y + height

                            recentsPopup.open();
                        }
                    }

                    ComboBox {
                        id: fileSelector
                        textRole: "fileName"
                        model: filesModel
                        Layout.fillWidth: true

                        currentIndex: fileView.currentIndex
                        onActivated: {
                            fileView.currentIndex = index
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
                                    playerView.player.source = videoPath;
                                    playerView.player.playPaused(0);
                                }
                            }

                        } else {
                            var dvRescueXmlPath = filePath + dvRescueXmlExtension
                            if(FileUtils.exists(dvRescueXmlPath))
                            {
                                refreshTimer.start();
                                dataModel.populate(dvRescueXmlPath);

                                segmentDataViewWithToolbar.segmentDataView.populateSegmentData(dvRescueXmlPath, filePath)
                            } else {
                                busy.running = true;
                                dvrescue.makeReport(filePath).then(() => {
                                                                       busy.running = false;
                                                                       refreshTimer.start();
                                                                       dataModel.populate(dvRescueXmlPath);

                                                                       if(currentIndex !== -1 && currentIndex !== undefined) {
                                                                           console.debug('reportPath resolved: ', dvRescueXmlPath)
                                                                           filesModel.setProperty(currentIndex, 'reportPath', dvRescueXmlPath)

                                                                           //filesModel.get(currentIndex).reportPath = dvRescueXmlPath;

                                                                           var mediaInfo = fileView.mediaInfoAt(currentIndex)
                                                                           mediaInfo.reportPath = dvRescueXmlPath;
                                                                           mediaInfo.resolve();

                                                                           segmentDataViewWithToolbar.segmentDataView.populateSegmentData(mediaInfo.reportPath, mediaInfo.videoPath)
                                                                       }
                                                                   }).catch((error) => {
                                                                        busy.running = false;
                                                                    });
                            }

                            playerView.player.source = filePath;
                            playerView.player.playPaused(0);
                        }
                    }
                }

                StackLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    currentIndex: fileSegmentSwitch.currentIndex

                    AnalyseFileView {
                        id: fileView
                        currentIndex: fileSelector.currentIndex

                        onFileAdded: {
                            root.recentFilesModel.addRecent(filePath)
                        }

                        onSelectedPathChanged: {
                            console.debug('selected path: ', selectedPath)
                            segmentDataViewWithToolbar.currentIndex = -1
                            segmentDataViewWithToolbar.hoveredItem = null
                            root.startFrame = 0;
                            root.endFrame = dataModel.total - 1;
                            toolsLayout.load(selectedPath, fileView.currentIndex)
                        }
                    }

                    SegmentDataViewWithToolbar {
                        id: segmentDataViewWithToolbar
                        framesCount: root.framesCount
                        reportPath: fileView.currentIndex !== -1 ? fileView.mediaInfoAt(fileView.currentIndex).reportPath : null
                        videoPath: fileView.currentIndex !== -1 ? fileView.mediaInfoAt(fileView.currentIndex).videoPath : null

                        segmentDataView.onClicked: {
                            console.debug('clicked: ', index, JSON.stringify(item));
                            if(index !== currentIndex) {
                                currentIndex = -1;
                                root.startFrame = 0;
                                root.endFrame = root.framesCount - 1;

                                dataView.invalidateFilter();
                            }
                        }

                        segmentDataView.onDoubleClicked: {
                            console.debug('doubleclicked: ', index, JSON.stringify(item));
                            if(index !== currentIndex) {
                                currentIndex = index;
                                root.startFrame = item['Frame #']
                                root.endFrame = root.framesCount - 1;
                                if((currentIndex + 1) < segmentDataView.model.rowCount) {
                                    root.endFrame = segmentDataView.model.getRow(currentIndex + 1)['Frame #']
                                }

                                dataView.invalidateFilter();
                            }
                        }
                    }
                }
            }

            AnalyseDataView {
                id: dataView
                cppDataModel: dataModel
                ranges: segmentDataViewWithToolbar.hoveredItem && segmentDataViewWithToolbar.currentIndex === -1 ? segmentDataViewWithToolbar.hoveredItem.range : Qt.vector2d(-1, -1)
                rowFilter: function(index) {
                    var data = dataView.model.getRow(index);
                    var frame = data['Frame #'];
                    if(frame < root.startFrame)
                        return false;

                    if(frame > root.endFrame)
                        return false;

                    return true;
                }

                DvLoupeView {
                    id: dvloupeView
                    property int index: -1

                    canPrev: index > 0
                    canNext: index < (dataView.model.rowCount - 1)
                    onPrev: {
                        --index
                        imageSource = null
                        fetch()
                    }
                    onNext: {
                        ++index
                        imageSource = null
                        fetch()
                    }

                    onSelectionChanged: {
                        var selectedRows = [];
                        for(var i = 0; i < dvloupeView.dataModel.rowCount; ++i) {
                            var rowData = dvloupeView.dataModel.getRow(i);
                            if(rowData.selected)
                                selectedRows.push(i);
                        }

                        var data = dataView.model.getRow(index);
                        var offset = data['Byte Offset']

                        imageSource = null

                        doDvPlay(offset, selectedRows)
                    }

                    function doDvPlay(offset, selection) {
                        console.debug('executing dvplay... ');

                        var extra = ''
                        if(selection && selection.length !== 0) {
                            extra = ' -B ' + selection.join(',') + ' '
                        }

                        dvplay.exec('-O - -b' + ' ' + offset + ' ' + extra + playerView.player.source).then((result) => {
                            var dataUri = ImageUtils.toDataUri(result.output, "png");
                            if(LoggingUtils.isDebugEnabled(dvplay.dvplayCategory.name)) {
                                console.debug(dvplay.dvplayCategory, 'got dataUri from dvplay: ', dataUri)
                            }
                            dvloupeView.imageSource = dataUri
                        }).catch((err) => {
                            console.error('dvplay.exec error: ', err)
                        })
                    }

                    function fetch() {
                        var data = dataView.model.getRow(index);
                        var offset = data['Byte Offset']

                        doDvPlay(offset)

                        console.debug('executing dvloupe... ');

                        var extraParams = " -M {mediainfo} -x {xml} -F {ffmpeg}"
                        .replace("{mediainfo}", dvloupe.effectiveMediaInfoCmd)
                        .replace("{xml}", dvloupe.effectiveXmlStarletCmd)
                        .replace("{ffmpeg}", dvloupe.effectiveFfmpegCmd)

                        dvloupe.exec('-i' + ' ' + playerView.player.source + ' ' + '-b' + ' ' + offset + ' -f json -T n', (launcher) => {
                            debugView.logCommand(launcher)
                        }, extraParams).then((result) => {
                            dvloupeView.data = JSON.parse(result.outputText)
                        }).catch((err) => {
                            console.error('dvloupe.exec error: ', err)
                        })
                    }
                }

                DvPlayCtl {
                    id: dvplay

                    xmlStarletCmd: settings.xmlStarletCmd
                    mediaInfoCmd: settings.mediaInfoCmd
                    ffmpegCmd: settings.ffmpegCmd

                    Component.onCompleted: {
                        if(Qt.platform.os === "windows") {
                            paths = [ FileUtils.getFileDir(settings.dvrescueCmd), FileUtils.getFileDir(settings.xmlStarletCmd),
                                                  FileUtils.getFileDir(settings.mediaInfoCmd), FileUtils.getFileDir(settings.ffmpegCmd) ]
                        }
                    }
                }

                DvLoupeCtl {
                    id: dvloupe

                    xmlStarletCmd: settings.xmlStarletCmd
                    mediaInfoCmd: settings.mediaInfoCmd
                    ffmpegCmd: settings.ffmpegCmd

                    Component.onCompleted: {
                        if(Qt.platform.os === "windows") {
                            paths = [ FileUtils.getFileDir(settings.dvrescueCmd), FileUtils.getFileDir(settings.xmlStarletCmd),
                                                  FileUtils.getFileDir(settings.mediaInfoCmd), FileUtils.getFileDir(settings.ffmpegCmd) ]
                        }
                    }
                }

                onFrameInfoRequested: {

                    dvloupeView.index = index
                    dvloupeView.fetch()
                    dvloupeView.open()
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
