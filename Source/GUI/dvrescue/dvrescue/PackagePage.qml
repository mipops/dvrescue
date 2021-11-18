import QtQuick 2.0
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import FileUtils 1.0
import Qt.labs.platform 1.1
import Launcher 0.1
import QtQuick.Controls 1.4 as QQC1

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
                                      var entries = FileUtils.ls(FileUtils.getFileDir(filePath));
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

        anchors.left: parent.left
        anchors.right: parent.right

        property string dvRescueXmlExtension: ".dvrescue.xml"
        property int fileViewerHeight: 0

        CustomButton {
            id: addFiles
            icon.source: "icons/add-files.svg"
            onClicked: {
                selectPath.callback = (urls) => {
                    urls.forEach((url) => {
                                     var filePath = FileUtils.getFilePath(url);
                                     filesModel.add(filePath);
                                     root.recentFilesModel.addRecent(filePath)
                                 });
                }

                selectPath.open();
            }
        }

        CustomButton {
            icon.source: "icons/recent.svg"

            onClicked: {
                var mapped = mapToItem(root, 0, 0);
                recentsPopup.x = mapped.x
                recentsPopup.y = mapped.y + height

                recentsPopup.open();
            }
        }

        Rectangle {
            Layout.fillWidth: true
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

                Button {
                    text: "Package"
                    height: parent.height
                    onClicked: {
                        for(var i = 0; i < packageOutputFileView.dataModel.rowCount; ++i) {
                            packageOutputFileView.updatePackagingStatus(i, "queued");
                        }

                        var packagingPath = ''
                        var opts = { 'type' : mov.checked ? 'mov' : 'mkv' }
                        var promise = segmentDataViewWithToolbar.segmentDataView.packaging(reportPath, videoPath, outputPath, opts, (o) => {
                            var splitted = String(o).split('\n');
                            for(var i = 0; i < splitted.length; ++i) {
                                var value = splitted[i];
                                console.debug('value:', value);

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
                        }).catch((err) => {
                            console.error('packaging failed: ', err);
                        })
                    }
                }

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
            packageOutputFileView.height = height / 5 * 2
            segmentDataViewWithToolbar.height = height / 5 * 2
            fileView.height = height / 5
        }

        PackageFileView {
            id: fileView

            onFileAdded: {
                root.recentFilesModel.addRecent(filePath)
            }
        }

        SegmentDataViewWithToolbar {
            id: segmentDataViewWithToolbar

            framesCount: root.framesCount
            reportPath: root.reportPath
            onReportPathChanged: {
                console.debug('reportPath changed: ', reportPath)
                // need to execute it after outputPath updated
                Qt.callLater(() => { segmentDataView.populateSegmentData(reportPath, videoPath, outputPath) })
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
                packageOutputFileView.dataModel.clear();
                for(var i = 0; i < segmentDataView.model.rowCount; ++i) {
                    var row = segmentDataView.model.getRow(i)
                    packageOutputFileView.newRow(row['FileName'])
                }
            }
        }

        PackageOutputFileView {
            id: packageOutputFileView

            function updatePackagingErrorByPath(path, error) {
                updatePropertyByPath(path, 'Error', error)
            }

            function updatePackagingStatusByPath(path, status) {
                updatePropertyByPath(path, 'Status', status)
            }

            function updatePackagingError(index, error) {
                updateProperty(index, 'Error', error)
            }

            function updatePackagingStatus(index, status) {
                updateProperty(index, 'Status', status)
            }

            function updatePropertyByPath(path, propertyName, value) {
                for(var j = 0; j < dataModel.rowCount; ++j) {
                    var row = dataModel.getRow(j)
                    if((row['File Path']) === path) {
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
