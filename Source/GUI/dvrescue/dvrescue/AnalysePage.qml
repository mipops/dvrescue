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

    signal commandExecutionStarted(var launcher);
    signal commandExecutionFinished(var results);

    property int startFrame: 0
    onStartFrameChanged: {
        console.debug('analyse page: startFrame = ', startFrame)
    }

    property int endFrame: dataModel.total - 1
    onEndFrameChanged: {
        console.debug('analyse page: endFrame = ', endFrame)
    }

    property alias dataView: dataView
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
        evenVideoCurve: plotsView.evenVideoCurve
        oddVideoCurve: plotsView.oddVideoCurve
        evenAudioCurve: plotsView.evenAudioCurve
        oddAudioCurve: plotsView.oddAudioCurve

        onTotalChanged: {
            console.debug('total changed: ', total)
        }

        onPopulated: {
            console.debug('stopping timer')
            refreshTimer.stop();

            root.startFrame = 0;
            root.endFrame = dataModel.total - 1;
            dataView.invalidateFilter();

            dataModel.update();
        }
        onError: {
            errorDialog.text = "error message: " + errorString
            errorDialog.open()
        }

        Component.onCompleted: {
            dataModel.update();
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
            var dirPath = FileUtils.getFileDir(filePath)
            if (!FileUtils.isWritable(dirPath)) {
                FileUtils.requestRWPermissionsForPath(dirPath, qsTr("Please authorize DVRescue to write to the containing folder to proceed."));
            }
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

            preferredWidth: splitView.width / 2

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
                fileViewColumn.preferredHeight = height / 5 * 1.5
            }

            ColumnLayout {
                id: fileViewColumn
                spacing: 0

                property int preferredHeight: 0
                onPreferredHeightChanged: {
                    SplitView.preferredHeight = preferredHeight
                }

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

                    function load(fileInfo, currentIndex) {

                        console.debug('load: ', JSON.stringify(fileInfo, 0, 4), 'currentIndex: ', currentIndex)
                        dataModel.reset();

                        if(fileInfo && fileInfo.videoPath) {
                            playerView.player.source = fileInfo.videoPath;
                            playerView.player.playPaused(0);
                        } else {
                            playerView.player.source = '';
                        }

                        if(fileInfo && fileInfo.reportPath) {
                            refreshTimer.start();

                            dataModel.populate(fileInfo.reportPath);
                            if(fileInfo.videoPath) {
                                segmentDataViewWithToolbar.segmentDataView.populateSegmentData(fileInfo.reportPath, fileInfo.videoPath)
                            }
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

                            if(selectedPath === null)
                                return;

                            var path = selectedPath
                            if(dvrescue.pendingReports.hasOwnProperty(path)) {
                                dvrescue.pendingReports[path].then((reportPath) => {
                                                          var fileInfo = filesModel.infoByPath(path)
                                                          console.debug('delayed reportInfo: ', path, JSON.stringify(fileInfo))
                                                          toolsLayout.load(fileInfo, fileView.currentIndex)
                                                      })
                            } else {
                                var fileInfo = filesModel.infoByPath(path)
                                toolsLayout.load(fileInfo, fileView.currentIndex)
                            }
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

                        var extra = []
                        if(selection && selection.length !== 0) {
                            extra.push(...['-B', selection.join(',')])
                        }

                        var args = ['-O', '-', '-b', offset]
                        args.push(...extra)
                        args.push(playerView.player.source)
                        dvplay.exec(args).then((result) => {
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

                        var extraParams = ['-M', dvloupe.effectiveMediaInfoCmd]
                        extraParams.push(...['-x', dvloupe.effectiveXmlStarletCmd])
                        extraParams.push(...['-F', dvloupe.effectiveFfmpegCmd])

                        var filterOptions = []
                        if(headerCheckboxChecked) {
                            filterOptions.push('-H')
                        }

                        if(subcodeCheckboxChecked) {
                            filterOptions.push('-S')
                        }

                        if(vauxCheckboxChecked) {
                            filterOptions.push('-X')
                        }

                        if(audioCheckboxChecked) {
                            filterOptions.push('-A')
                        }

                        if(videoCheckboxChecked) {
                            filterOptions.push('-V')
                        }

                        if(errorOnlyCheckboxChecked) {
                            filterOptions.push('-E')
                        }

                        var args = ['-i', playerView.player.source, '-b', offset, '-f', 'json', '-T', 'n']
                        filterOptions.forEach((opt) => args.push(opt))
                        dvloupe.exec(args, (launcher) => {
                            commandExecutionStarted(launcher)
                        }, extraParams).then((result) => {
                            commandExecutionFinished(result.outputText)
                            dvloupeView.data = JSON.parse(result.outputText)
                        }).catch((err) => {
                            commandExecutionFinished(err)
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
            dataModel.update();
        }
    }
}
