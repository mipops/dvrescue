import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import Qt.labs.settings 1.0
import Qt.labs.platform 1.1
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
                                      var dirPath = FileUtils.getFileDir(filePath)

                                      // Test if we can access content directory, or open a dialog to try to gains rights on it
                                      if (!FileUtils.isWritable(dirPath)) {
                                          FileUtils.requestRWPermissionsForPath(dirPath, qsTr("Please authorize DVRescue to write to the containing folder to proceed."));
                                      }
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

    FolderDialog {
        id: selectFolderDialog
        property var callback;

        onAccepted: {
            var folderUrl = selectFolderDialog.currentFolder;
            console.debug('selected folder: ', folderUrl);
            if(callback)
                callback(folderUrl);
        }
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

            preferredWidth: playerView.width

            PlayerView {
                id: playerView

                Component.onCompleted: {
                    SplitView.preferredHeight = Qt.binding(function() { return height })
                    height = Qt.binding(() => { return playerAndPlotsSplitView.height / 2 })
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

                function seekToFrame(frameIndex, bringToView) {
                    var frameOffset = dataModel.frameOffset(frameIndex);
                    if(frameOffset !== -1) {
                        console.debug('seeking to: ', frameIndex, frameOffset);
                        player.waitForSeekFinished().then(() => {
                             console.debug('seekToFrame finished: ', player.position)
                             if(!bringToView)
                                dataView.skipBringToView = true
                             positionChanged(frameIndex)
                             if(!bringToView)
                                dataView.skipBringToView = false

                             player.notifyPositionChanged();
                             player.startTrackPosition();
                             notifyPositionUpdates = true
                        });
                        notifyPositionUpdates = false
                        player.stopTrackPosition();
                        player.seek(frameOffset);
                    }
                }

                property bool notifyPositionUpdates: true
                property int prevDisplayPosition: -1
                player.onPositionChanged: {
                    var displayPosition = QtAVPlayerUtils.displayPosition(player)
                    if(prevDisplayPosition === displayPosition)
                        return;

                    console.debug('player.onPositionChanged: ', displayPosition)
                    prevDisplayPosition = displayPosition;

                    var frameIndex = dataModel.frameIndex(displayPosition);

                    if(notifyPositionUpdates) {
                        console.debug('player.displayPosition: ', displayPosition, 'frameIndex: ', frameIndex)
                        playerView.positionChanged(frameIndex);
                    }
                }

                Connections {
                    target: dataView
                    onTapped: {
                        playerView.seekToFrame(framePos, false)
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
                    playerView.seekToFrame(frameIndex, true);
                }

                onMarkerClicked: {
                    playerView.seekToFrame(frameIndex, true);
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

                    Item {
                        height: parent.height
                        width: 1
                    }

                    CustomButton {
                        icon.color: "transparent"
                        icon.source: "/icons/add-files.svg"
                        implicitHeight: 30
                        implicitWidth: 47

                        onClicked: {
                            selectPath.callback = (urls) => {
                                urls.forEach((url) => {
                                                var filePath = FileUtils.getFilePath(url);
                                                var dirPath = FileUtils.getFileDir(filePath)

                                                // Test if we can access content directory, or open a dialog to try to gains rights on it
                                                if (!FileUtils.isWritable(dirPath)) {
                                                    FileUtils.requestRWPermissionsForPath(dirPath, qsTr("Please authorize DVRescue to write to the containing folder to proceed."));
                                                }
                                                filesModel.add(filePath)
                                             });
                            }

                            selectPath.open();
                        }
                    }

                    CustomButton {
                        icon.color: "transparent"
                        icon.source: "/icons/recent.svg"
                        implicitHeight: 30
                        implicitWidth: 47

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
                property bool skipBringToView: false

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
                    x: (parent.width - width) / 2
                    y: (parent.height - height) / 2

                    property int index: -1

                    canPrev: index > 0
                    canNext: index < (dataView.model.rowCount - 1)
                    onPrev: {
                        --index
                        fetch()
                    }
                    onNext: {
                        ++index
                        fetch()
                    }
                    onRefresh: {
                        fetch('dvloupe');
                    }

                    onSelectionChanged: {
                        var selectedVblRows = [];
                        for(var i = 0; i < dvloupeView.dataModel.rowCount; ++i) {
                            var rowData = dvloupeView.dataModel.getRow(i);
                            if(rowData.selected)
                                selectedVblRows.push(rowData.vbl);
                        }

                        var data = dataView.model.getRow(index);
                        var offset = data['Byte Offset']

                        doDvPlay(offset, selectedVblRows)
                    }

                    function doDvPlay(offset, selection) {
                        console.debug('executing dvplay... ');
                        showDvLoupeBusyIndicator = true

                        var extra = ''
                        if(selection && selection.length !== 0) {
                            extra = ' -B ' + selection.join(',') + ' '
                        }

                        dvplay.exec('-O - -b' + ' ' + offset + ' ' + extra + playerView.player.source).then((result) => {
                            var dataUri = ImageUtils.toDataUri(result.output, "jpg");
                            if(LoggingUtils.isDebugEnabled(dvplay.dvplayCategory.name)) {
                                console.debug(dvplay.dvplayCategory, 'got dataUri from dvplay: ', dataUri)
                            }
                            dvloupeView.imageSource = dataUri
                            showDvLoupeBusyIndicator = false
                        }).catch((err) => {
                            console.error('dvplay.exec error: ', err)
                            showDvLoupeBusyIndicator = false
                        })
                    }

                    function fetch(args) {
                        var data = dataView.model.getRow(index);
                        var offset = data['Byte Offset']

                        if(args !== 'dvloupe') {
                            doDvPlay(offset)
                        }

                        console.debug('executing dvloupe... ');

                        var extraParams = " -M {mediainfo} -x {xml} -F {ffmpeg}"
                        .replace("{mediainfo}", dvloupe.effectiveMediaInfoCmd)
                        .replace("{xml}", dvloupe.effectiveXmlStarletCmd)
                        .replace("{ffmpeg}", dvloupe.effectiveFfmpegCmd)

                        var filterOptions = ''
                        if(headerCheckboxChecked) {
                            filterOptions += ' -H'
                        }

                        if(subcodeCheckboxChecked) {
                            filterOptions += ' -S'
                        }

                        if(vauxCheckboxChecked) {
                            filterOptions += ' -X'
                        }

                        if(audioCheckboxChecked) {
                            filterOptions += ' -A'
                        }

                        if(videoCheckboxChecked) {
                            filterOptions += ' -V'
                        }

                        if(errorOnlyCheckboxChecked) {
                            filterOptions += ' -E'
                        }

                        dvloupe.exec('-i' + ' ' + playerView.player.source + ' ' + '-b' + ' ' + offset + ' -f json -T n' + filterOptions, (launcher) => {
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
                        if(!dataView.skipBringToView)
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
