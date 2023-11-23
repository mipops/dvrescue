import QtQuick 2.0
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import FileUtils 1.0
import Qt.labs.platform 1.1
import Launcher 0.1
import SplitView 1.0
import TableModel 1.0
import TableModelColumn 1.0
import Qt.labs.qmlmodels 1.0
import QtQuick 2.12
import CsvParser 0.1

Item {
    id: root

    signal commandExecutionStarted(var launcher);
    signal commandExecutionFinished(var results);

    property string dvrescueCmd
    property string xmlStarletCmd
    property string mediaInfoCmd
    property string ffmpegCmd
    property alias filesModel: fileView.filesModel
    property alias mergeInputFilesView: mergeInputFileView
    property alias recentFilesModel: recentsPopup.filesModel
    property alias csvParser: csvParser
    property alias mergeReportView: mergeReportView

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

    SelectPathDialog {
        id: selectPath
        selectMultiple: true
        nameFilters: [
            "Report and video files (*.dvrescue.xml *.mov *.mkv *.avi *.dv *.mxf)",
            "Report files (*.dvrescue.xml)",
            "Video files (*.mov *.mkv *.avi *.dv *.mxf)"
        ]
    }

    SelectPathDialog {
        id: selectFile
        selectMultiple: false
        selectExisting: false
        nameFilters: [
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

    SplitView {
        id: splitView
        anchors.fill: parent
        anchors.margins: 20
        orientation: Qt.Horizontal
        onWidthChanged: {
            leftLayout.width = width / 2
        }

        SplitView {
            id: leftLayout
            orientation: Qt.Vertical

            Component.onCompleted: {
                SplitView.preferredWidth = Qt.binding(function() { return width })
            }

            onHeightChanged: {
                selectFilesPanel.height = height / 5 * 1.5
            }

            Rectangle {
                id: selectFilesPanel
                color: 'white'

                Component.onCompleted: {
                    SplitView.preferredHeight = Qt.binding(function() { return height })
                }

                ButtonGroup {
                    buttons: [packageIntoSameFolderButton, specifyPathButton]
                }

                ColumnLayout {
                    anchors.centerIn: parent

                    RadioButton {
                        id: packageIntoSameFolderButton
                        text: 'Package into same folder'
                        checked: true
                    }

                    RowLayout {
                        RadioButton {
                            id: specifyPathButton
                            text: 'Specify path'
                        }

                        TextField {
                            id: filePath
                            placeholderText: 'path...'
                            implicitWidth: 400
                        }

                        ToolButton {
                            onClicked: {
                                selectFile.callback = (url) => {
                                    var file = FileUtils.getFilePath(url);
                                    filePath.text = file;
                                }
                                selectFile.open();
                            }
                        }
                    }

                    Button {
                        text: 'Merge'
                        enabled: mergeInputFileView.dataModel.rowCount !== 0
                        onClicked: {
                            var inputFiles = [];
                            for(var i = 0; i < mergeInputFileView.dataModel.rowCount; ++i) {
                                inputFiles.push(mergeInputFileView.dataModel.getRow(i)[mergeInputFileView.filePathColumn])
                            }

                            var outputFile = '';

                            if(specifyPathButton.checked && filePath.text !== '') {
                                outputFile = filePath.text
                            }

                            if(packageIntoSameFolderButton.checked) {
                                outputFile = inputFiles[0] + "_merged.dv";
                            }

                            if(FileUtils.exists(outputFile)) {
                                FileUtils.remove(outputFile);
                            }

                            mergeOutputFileView.newRow(outputFile);
                            mergeOutputFileView.updatePackagingStatusByPath(outputFile, 'packaging')

                            dvrescue.merge(inputFiles, outputFile, (launcher) => {
                                commandExecutionStarted(launcher);
                            }).then((result) => {
                                csvParser.rows = [];
                                csvParser.write(result.outputText);

                                mergeReportView.refresh();
                                mergeOutputFileView.updatePackagingStatusByPath(outputFile, 'finished')
                                commandExecutionFinished(result.outputText);
                            })
                            .catch((error) => {
                                packageOutputFileView.updatePackagingErrorByPath(outputFile, error);
                                commandExecutionFinished(error);
                            })
                        }
                    }
                }
            }

            Rectangle {
                id: summaryTablePanel
                color: 'transparent'

                CsvParser {
                    id: csvParser

                    property int indexOfFramePos: -1
                    property int indexOftc: -1
                    property int indexOfUsed: -1
                    property int indexOfStatus: -1
                    property int indexOfIssueFixed: -1
                    property int indexOfBlockErrors: -1
                    property int indexOfComments: -1

                    property var rows: []

                    onColumnsChanged: {

                        var columnNames = columns;
                        console.debug('columnNames: ', JSON.stringify(columnNames))

                        //"Frame #" // FramePos
                        //"Timecode" // tc
                        //"File Selection" // Used
                        //"Status"
                        //"IssueFixed"
                        //"BlockErrors"
                        //"Comments"

                        indexOfFramePos = columnNames.indexOf('FramePos');
                        indexOftc = columnNames.indexOf('tc');
                        indexOfUsed = columnNames.indexOf('Used');
                        indexOfStatus = columnNames.indexOf('Status');
                        indexOfIssueFixed = columnNames.indexOf('IssueFixed');
                        indexOfBlockErrors = columnNames.indexOf('BlockErrors');
                        indexOfComments = columnNames.indexOf('Comments');

                        console.debug('indexOfFramePos: ', indexOfFramePos)
                        console.debug('indexOftc: ', indexOftc)
                        console.debug('indexOfUsed: ', indexOfUsed)
                        console.debug('indexOfStatus: ', indexOfStatus)
                        console.debug('indexOfIssueFixed: ', indexOfIssueFixed)
                        console.debug('indexOfBlockErrors: ', indexOfBlockErrors)
                        console.debug('indexOfComments: ', indexOfComments)
                    }

                    onEntriesReceived: {
                        var framePos = entries[indexOfFramePos]
                        var tc = entries[indexOftc]
                        var used = entries[indexOfUsed]
                        var status = entries[indexOfStatus]
                        var issueFixed = entries[indexOfIssueFixed]
                        var blockErrors = entries[indexOfBlockErrors]
                        var comments = entries[indexOfComments]

                        rows.push({
                                      'Frame #' : framePos,
                                      'Timecode' : tc,
                                      'File Selection' : used,
                                      'Status' : status,
                                      'IssueFixed' : issueFixed,
                                      'BlockErrors' : blockErrors,
                                      'Comments' : comments
                                  })
                    }
                }

                MergeReportView {
                    id: mergeReportView
                    anchors.fill: parent

                    function refresh() {
                        dataModel.clear();

                        for(var i = 0; i < csvParser.rows.length; ++i) {
                            var used = csvParser.rows[i][fileSelectionColumn];
                            if(i === 0 || i === (csvParser.rows.length - 1) || used !== '0')
                                dataModel.appendRow(csvParser.rows[i])
                        }
                    }

                    Component.onCompleted: {

                    }
                }
            }
        }

        SplitView {
            id: rightLayout
            orientation: Qt.Vertical

            onHeightChanged: {
                filesPanel.height = height / 5 * 2
                mergeInputFileView.height = height / 5 * 2
                mergeOutputFileView.height = height / 5 * 1
            }

            Item {
                id: filesPanel
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

                MergeFileView {
                    id: fileView
                    anchors.top: toolsLayout.bottom
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                }

                Component.onCompleted: {
                    SplitView.preferredHeight = Qt.binding(function() { return height })
                }
            }

            MergeInputFileView {
                id: mergeInputFileView

                Component.onCompleted: {
                    SplitView.preferredHeight = Qt.binding(function() { return height })
                }
            }

            MergeOutputFileView {
                id: mergeOutputFileView

                Component.onCompleted: {
                    SplitView.preferredHeight = Qt.binding(function() { return height })
                }
            }

            /*
            Item {
                id: activityLogPanel
                Text {
                    id: label
                    text: 'Activity log'
                    color: 'white'
                    font.pixelSize: 20
                }

                Rectangle {
                    color: 'white'
                    anchors.top: label.bottom
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.bottom:  parent.bottom

                    TextEdit {
                        anchors.fill: parent
                    }
                }
            }
            */
        }
    }
}
