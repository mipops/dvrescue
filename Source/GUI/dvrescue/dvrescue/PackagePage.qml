import QtQuick 2.0
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import FileUtils 1.0
import Qt.labs.platform 1.1
import Launcher 0.1
import SplitView 1.0

Item {
    id: root

    property string dvrescueCmd
    property string xmlStarletCmd
    property string mediaInfoCmd
    property string ffmpegCmd
    property alias filesModel: fileView.filesModel
    property alias recentFilesModel: recentsPopup.filesModel
    property int framesCount

    property string videoPath: fileView.currentIndex !== -1 ? filesModel.get(fileView.currentIndex).videoPath : null
    onVideoPathChanged: {
        console.debug('root: videoPath = ', videoPath)
    }

    property string outputPath: packageIntoSameFolder.checked ? (videoPath ? FileUtils.getFileDir(videoPath) : null) : customPackagingPath.text
    onOutputPathChanged: {
        console.debug('root: outputPath = ', outputPath)
    }

    property string reportPath: fileView.currentIndex !== -1 ? filesModel.get(fileView.currentIndex).reportPath : null
    onReportPathChanged: {
        console.debug('root: reportPath = ', reportPath)
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
                                      var dirPath = FileUtils.getFileDir(filePath);

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
                // Test if we can access content directory, or open a dialog to try to gains rights on it
                var dirPath = FileUtils.getFileDir(filePath)
                if (!FileUtils.isWritable(dirPath)) {
                    FileUtils.requestRWPermissionsForPath(dirPath, qsTr("Please authorize DVRescue to write to the containing folder to proceed."));
                }

            root.filesModel.add(filePath)
        }
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

    function toNativePath(path) {
        if(Qt.platform.os === "windows") {
           var cygwinPath = path;
           var splittedWinPath = cygwinPath.replace('/cygdrive/', '').split('/')
           if(splittedWinPath.length !== 0) {
             splittedWinPath[0] = splittedWinPath[0] + ':'
             path = splittedWinPath.join('\\')
           }
        }
        return path;
    }

    RowLayout {
        id: toolsLayout

        property string dvRescueXmlExtension: ".dvrescue.xml"
        property int fileViewerHeight: 0

        CustomButton {
            id: addFiles
            icon.color: "transparent"
            icon.source: "/icons/add-files.svg"
            implicitHeight: 30
            implicitWidth: 47

            onClicked: {
                selectPath.callback = (urls) => {
                    urls.forEach((url) => {
                                     var filePath = FileUtils.getFilePath(url);
                                     var dirPath = FileUtils.getFileDir(filePath);

                                     // Test if we can access content directory, or open a dialog to try to gains rights on it
                                     if (!FileUtils.isWritable(dirPath)) {
                                         FileUtils.requestRWPermissionsForPath(dirPath, qsTr("Please authorize DVRescue to write to the containing folder to proceed."));
                                     }
                                     filesModel.add(filePath);
                                     root.recentFilesModel.addRecent(filePath)
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
                recentsPopup.x = mapped.x
                recentsPopup.y = mapped.y + height

                recentsPopup.open();
            }
        }

    }


    SplitView {
        id: splitView
        anchors.top: toolsLayout.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        orientation: Qt.Vertical

        onHeightChanged: {
            packageOutputFileView.height = height / 5 * 2
            segmentDataViewWithToolbar.height = height / 5 * 2
            fileView.height = height / 5
        }

        PackageFileView {
            id: fileView

            Component.onCompleted: {
                SplitView.preferredHeight = Qt.binding(function() { return height })
            }

            onFileAdded: {
                root.recentFilesModel.addRecent(filePath)
            }

            onSelectedPathChanged: {
                console.debug('PackageFileView.selectedPath: ', selectedPath);

                Qt.callLater(() => {
                                 if(dvrescue.pendingReports.hasOwnProperty(selectedPath)) {
                                    var promise = dvrescue.pendingReports[selectedPath]
                                    busy.running = true
                                    promise.then(() => {
                                         busy.running = false
                                    }).catch((err) => {
                                         busy.running = false
                                    })
                                 }
                             });

            }
        }

        SegmentDataViewWithToolbar {
            id: segmentDataViewWithToolbar
            property int updatesCounter: 0

            Component.onCompleted: {
                SplitView.preferredHeight = Qt.binding(function() { return height })
            }

            framesCount: root.framesCount
            reportPath: root.reportPath
            onReportPathChanged: {
                console.debug('reportPath changed: ', reportPath)
                // need to execute it after outputPath updated
                Qt.callLater(() => {
                                    segmentDataView.populateSegmentData(reportPath, videoPath, outputPath)
                })
            }

            videoPath: root.videoPath
            onVideoPathChanged: {
                console.debug('videoPath changed: ', videoPath)
            }

            outputPath: root.outputPath
            onOutputPathChanged: {
                console.debug('outputPath changed: ', outputPath)
            }

            onPopulated: {
                ++updatesCounter
            }
        }

        PackageOutputFileView {
            id: packageOutputFileView

            Rectangle {
                y: -height * 1.1
                width: parent.width
                height: addFiles.height
                color: 'white'

                ButtonGroup {
                    buttons: [packageIntoSameFolder, specifyPath]
                }

                ButtonGroup {
                    buttons: [mov, mkv]
                }

                RowLayout {
                    id: settingsRow
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter

                    RadioButton {
                        id: mov
                        text: "mov"
                        checked: true
                        onCheckedChanged: {
                            if(checked) {
                                segmentDataViewWithToolbar.extraOpts = {'type' : 'mov'}
                            }
                        }
                    }

                    RadioButton {
                        id: mkv
                        text: "mkv"
                        onCheckedChanged: {
                            if(checked) {
                                segmentDataViewWithToolbar.extraOpts = {'type' : 'mkv'}
                            }
                        }
                    }

                    RadioButton {
                        id: packageIntoSameFolder
                        text: "Package into same folder"
                        checked: true
                    }

                    RadioButton {
                        id: specifyPath
                        text: "Specify path"
                    }

                    TextField {
                        enabled: specifyPath.checked
                        id: customPackagingPath
                        placeholderText: "path..."
                        Layout.minimumWidth: 200
                    }

                    ToolButton {
                        enabled: specifyPath.checked
                        text: "..."
                        onClicked: {
                            selectFolderDialog.callback = (selectedUrl) => {
                                customPackagingPath.text = FileUtils.getFilePath(selectedUrl)
                            };

                            selectFolderDialog.open()
                        }
                    }

                    Button {
                        id: addToQueue
                        text: "ADD TO QUEUE"
                        height: parent.height
                        property alias segmentDataView: segmentDataViewWithToolbar.segmentDataView
                        property bool isPackaging: false
                        property var segments: ({})
                        property int clickCounter: 0

                        function segmentsAlreadyAdded() {
                            for(var i = 0; i < segmentDataView.model.rowCount; ++i) {
                                var row = segmentDataView.model.getRow(i)
                                var fileName = row['FileName'];
                                if(segments.hasOwnProperty(fileName))
                                    return false
                            }

                            return true;
                        }

                        enabled: segmentDataViewWithToolbar.updatesCounter, clickCounter, segmentsAlreadyAdded()

                        onClicked: {
                            var timestamp = new Date();
                            for(var i = 0; i < segmentDataView.model.rowCount; ++i) {
                                var row = segmentDataView.model.getRow(i)
                                var fileName = row['FileName'];
                                var opts = { 'type' : mov.checked ? 'mov' : 'mkv' }
                                segments[fileName] = {
                                    'reportPath' : reportPath,
                                    'videoPath' : videoPath,
                                    'outputPath' : outputPath,
                                    'opts' : opts,
                                    'timestamp' : timestamp
                                }
                                packageOutputFileView.newRow(fileName)
                            }

                            var packageFunc = () => {
                                var indexes = []
                                var segment = null

                                for(var i = 0; i < packageOutputFileView.dataModel.rowCount; ++i) {
                                    var row = packageOutputFileView.dataModel.getRow(i)
                                    if(row[packageOutputFileView.statusColumn] === 'not exported') {
                                        var fileName = row[packageOutputFileView.filePathColumn]
                                        var currentSegment = segments[fileName];
                                        var newTimestamp = currentSegment['timestamp'];
                                        if(segment === null) {
                                            segment = currentSegment
                                        }
                                        else if(segment['timestamp'] === newTimestamp) {
                                            packageOutputFileView.updatePackagingStatus(i, "queued");
                                            indexes.push(i);
                                        }
                                        else {
                                            break;
                                        }
                                    }
                                }

                                if(segment === null)
                                    return;

                                isPackaging = true;
                                var packagingPath = ''
                                var opts = { 'type' : mov.checked ? 'mov' : 'mkv' }
                                var promise = segmentDataViewWithToolbar.segmentDataView.packaging(segment.reportPath, segment.videoPath,
                                                                                                   segment.outputPath, segment.opts, (o) => {
                                    console.debug('packaging output changed: ', o)

                                    var splitted = String(o).split('\n');
                                    for(var i = 0; i < splitted.length; ++i) {
                                        var value = splitted[i].trim();
                                        console.debug('packaging output splitted value:', value);

                                        if(value.startsWith('### Packaging started: ')) {
                                            var path = root.toNativePath(value.replace('### Packaging started: ', ''));
                                            packagingPath = path;
                                            packageOutputFileView.updatePackagingStatusByPath(path, 'packaging');
                                        } else if(value.startsWith('### Packaging finished: ')) {
                                            var path = root.toNativePath(value.replace('### Packaging finished: ', ''));
                                            packageOutputFileView.updatePackagingStatusByPath(path, 'finished');
                                        } else if(value.startsWith('### Packaging error: ')) {
                                            var error = value.replace('### Packaging error: ', '');
                                            packageOutputFileView.updatePackagingErrorByPath(packagingPath, error);
                                        }
                                    }
                                });
                                promise.then(() => {
                                    console.debug('packaging done');
                                    indexes.forEach((index) => {
                                                        console.debug('indexes: ', index)
                                                        var row = packageOutputFileView.dataModel.getRow(index)
                                                        var filePath = row[packageOutputFileView.filePathColumn]
                                                        delete segments.filePath

                                                        var status = row[packageOutputFileView.statusColumn]
                                                        if(status === 'packaging') {
                                                            packageOutputFileView.updatePackagingError(index, 'unexpected')
                                                            packageOutputFileView.updatePackagingStatus(index, 'finished')
                                                        }
                                                    })
                                    isPackaging = false;
                                    packageFunc()
                                }).catch((err) => {
                                    console.error('packaging failed: ', err);
                                    isPackaging = false;
                                    packageFunc()
                                })
                            }

                            if (!FileUtils.isWritable(outputPath))
                            {
                                selectFolderDialog.currentFolder = FileUtils.toLocalUrl(outputPath)
                                selectFolderDialog.callback = (selectedUrl) => {
                                    outputPath = FileUtils.getFilePath(selectedUrl)
                                    if(!isPackaging)
                                        packageFunc()
                                };
                                selectFolderDialog.open()
                            }
                            else
                            {
                                if(!isPackaging)
                                    packageFunc()
                            }

                            ++clickCounter
                        }
                    }

                    Button {
                        text: "CLEAR QUEUE"
                        height: parent.height
                        onClicked: {
                            packageOutputFileView.dataModel.clear()
                            addToQueue.segments = {}
                            ++addToQueue.clickCounter
                        }
                    }
                }
            }


            Component.onCompleted: {
                SplitView.preferredHeight = Qt.binding(function() { return height })
            }

            function updatePackagingErrorByPath(path, error) {
                console.debug('PackageOutputFileView: updatePackagingErrorByPath: ', path, error);
                updatePropertyByPath(path, 'Error', error)
            }

            function updatePackagingStatusByPath(path, status) {
                console.debug('PackageOutputFileView: updatePackagingStatusByPath: ', path, status);
                updatePropertyByPath(path, 'Status', status)
            }

            function updatePackagingError(index, error) {
                console.debug('PackageOutputFileView: updatePackagingError: ', index, error);
                updateProperty(index, 'Error', error)
            }

            function updatePackagingStatus(index, status) {
                console.debug('PackageOutputFileView: updatePackagingStatus: ', index, status);
                updateProperty(index, 'Status', status)
            }

            function updatePropertyByPath(path, propertyName, value) {
                for(var j = 0; j < dataModel.rowCount; ++j) {
                    var row = dataModel.getRow(j)
                    if((row['Output File Path']) === path) {
                        updateProperty(j, propertyName, value)
                        break;
                    }
                }
            }

            function updateProperty(index, propertyName, value) {
                var rowData = dataModel.getRow(index)
                var newRowData = JSON.parse(JSON.stringify(rowData))
                newRowData[propertyName] = value
                dataModel.setRow(index, newRowData)
            }
        }
    }
}
