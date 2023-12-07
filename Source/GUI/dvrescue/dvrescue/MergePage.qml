import QtQuick 2.0
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import FileUtils 1.0
import ImageUtils 1.0
import Qt.labs.platform 1.1
import Launcher 0.1
import LoggingUtils 1.0
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
    property alias summaryTablePanel: summaryTablePanel
    property alias recentFilesModel: recentsPopup.filesModel
    property alias csvParser: csvParser
    property alias mergeReportView: mergeReportView
    property alias mergeAnalyzeView: mergeAnalyzeView

    property var colors: 'red green blue magenta yellow cyan'.split(' ')

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

    MergeAnalyzeView {
        id: mergeAnalyzeView

        x: (parent.width - width) / 2
        y: (parent.height - height) / 2

        property int index: -1
        onIndexChanged: {
            var info = mergeReportView.dataModel.getRow(index);

            inputPos = info.InputPos;
            outputPos = info.OutputPos;
            timecodeText = info[mergeReportView.timeCodeColumn];
            frameNumberText = info[mergeReportView.framePosColumn];
        }

        property string inputPos: ''
        property string outputPos: ''

        canPrev: index > 0
        canNext: index < (mergeReportView.dataModel.rowCount - 1)
        onPrev: {
            --index
            fetch()
        }
        onNext: {
            ++index
            fetch()
        }
        onRefresh: {
            fetch()
        }

        function fetch() {
            doDvPlay(inputPos, mergeReportView.inputFiles, outputPos, mergeReportView.outputFile);
        }

        function doDvPlay(inputPos, inputFiles, outputPos, mergeResult) {
            console.debug('executing dvplay... ');
            showBusyIndicator = true

            if(inputPos.indexOf('|') === -1) {
                var inputPosArray = [];
                for(var i = 0; i < inputFiles.length; ++i) {
                    inputPosArray.push(inputPos);
                }
                inputPos = inputPosArray.join('|');
            }

            var extraParams = ['-M', dvplay.effectiveMediaInfoCmd]
            extraParams.push(...['-F', dvplay.effectiveFfmpegCmd])

            var args = ['-O', '-', '-S', inputPos, '-s', inputFiles.join('|'), '-b', outputPos, mergeResult]
            dvplay.exec(args, (launcher) => {
                commandExecutionStarted(launcher);
            }, extraParams).then((result) => {
                commandExecutionFinished('success');
                var dataUri = ImageUtils.toDataUri(result.output, "jpg");
                if(LoggingUtils.isDebugEnabled(dvplay.dvplayCategory.name)) {
                    console.debug(dvplay.dvplayCategory, 'got dataUri from dvplay: ', dataUri)
                }
                imageSource = dataUri
                showBusyIndicator = false
            }).catch((err) => {
                commandExecutionFinished(err);
                console.error('dvplay.exec error: ', err)
                showBusyIndicator = false
            })
        }
    }

    SplitView {
        id: splitView
        anchors.fill: parent
        anchors.margins: 20
        orientation: Qt.Vertical
        onHeightChanged: {
            topLayout.height = height / 3
        }

        SplitView {
            id: topLayout
            orientation: Qt.Horizontal

            Component.onCompleted: {
                SplitView.preferredHeight = Qt.binding(function() { return height })
            }

            onWidthChanged: {
                filesPanel.width = width / 2
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
                    SplitView.preferredWidth = Qt.binding(function() { return width })
                }
            }

            MergeInputFileView {
                id: mergeInputFileView
                colors: root.colors
                dropAreaEnabled: fileView.dragActive
                onDropAreaEnabledChanged: {
                    console.debug('dropAreaEnabled: ', dropAreaEnabled);
                }

                Component.onCompleted: {
                    SplitView.preferredHeight = Qt.binding(function() { return height })
                }
            }
        }

        SplitView {
            id: bottomLayout
            orientation: Qt.Horizontal

            Component.onCompleted: {
                SplitView.preferredHeight = Qt.binding(function() { return height })
            }

            onWidthChanged: {
                selectFilesAndMergeOutputLayout.width = width / 2
            }

            Item {
                id: selectFilesAndMergeOutputLayout

                Component.onCompleted: {
                    SplitView.preferredWidth = Qt.binding(function() { return width })
                }

                Rectangle {
                    id: selectFilesPanel
                    color: 'white'
                    anchors.left: parent.left
                    anchors.right: parent.right
                    height: buttonsColumn.childrenRect.height

                    ButtonGroup {
                        buttons: [packageIntoSameFolderButton, specifyPathButton]
                    }

                    ColumnLayout {
                        id: buttonsColumn
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

                                    mergeReportView.inputFiles = inputFiles;
                                    mergeReportView.outputFile = outputFile;
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

                MergeOutputFileView {
                    id: mergeOutputFileView
                    anchors.top: selectFilesPanel.bottom
                    anchors.bottom: parent.bottom
                    anchors.left: parent.left
                    anchors.right: parent.right
                }
            }

            Rectangle {
                id: summaryTablePanel
                color: 'transparent'

                Component.onCompleted: {
                    SplitView.preferredHeight = Qt.binding(function() { return height })
                }

                CsvParser {
                    id: csvParser

                    property int indexOfFramePos: -1
                    property int indexOftc: -1
                    property int indexOfUsed: -1
                    property int indexOfStatus: -1
                    property int indexOfIssueFixed: -1
                    property int indexOfBlockErrors: -1
                    property int indexOfBlockErrors_Even: -1
                    property int indexOfComments: -1

                    property int indexOfInputPos: -1
                    property int indexOfOutputPos: -1

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
                        indexOfBlockErrors_Even = columnNames.indexOf('BlockErrors_Even');
                        indexOfComments = columnNames.indexOf('Comments');
                        indexOfInputPos = columnNames.indexOf('InputPos');
                        indexOfOutputPos = columnNames.indexOf('OutputPos');

                        console.debug('indexOfFramePos: ', indexOfFramePos)
                        console.debug('indexOftc: ', indexOftc)
                        console.debug('indexOfUsed: ', indexOfUsed)
                        console.debug('indexOfStatus: ', indexOfStatus)
                        console.debug('indexOfIssueFixed: ', indexOfIssueFixed)
                        console.debug('indexOfBlockErrors: ', indexOfBlockErrors)
                        console.debug('indexOfBlockErrors_Even: ', indexOfBlockErrors_Even)
                        console.debug('indexOfComments: ', indexOfComments)
                        console.debug('indexOfInputPos: ', indexOfInputPos)
                        console.debug('indexOfOutputPos: ', indexOfOutputPos)
                    }

                    onEntriesReceived: {
                        var framePos = entries[indexOfFramePos]
                        var tc = entries[indexOftc]
                        var used = entries[indexOfUsed]
                        var status = entries[indexOfStatus]
                        var issueFixed = entries[indexOfIssueFixed]
                        var blockErrors = entries[indexOfBlockErrors]
                        var blockErrorsEven = entries[indexOfBlockErrors_Even]
                        var comments = entries[indexOfComments]
                        var inputPos = entries[indexOfInputPos]
                        var outputPos = entries[indexOfOutputPos]

                        var blockErrorsEvenOdd = Qt.point(Number(blockErrorsEven) / 1440 * 2,
                                                          (Number(blockErrors) - Number(blockErrorsEven)) / 1440 * 2);

                        if(Number(blockErrors) !== 0) {
                            console.debug('blockErrors: ', blockErrors, 'blockErrorsEven: ', blockErrorsEven)
                            console.debug('blockErrorsEvenOdd: ', blockErrorsEvenOdd)
                        }

                        rows.push({
                                      'Frame #' : framePos,
                                      'Timecode' : tc,
                                      'File Selection' : used,
                                      'Status' : status,
                                      'IssueFixed' : issueFixed,
                                      'BlockErrors' : blockErrors,
                                      'hasNoBlockErrors' : Number(blockErrors) === 0,
                                      'blockErrorsEvenOdd' : blockErrorsEvenOdd,
                                      'Comments' : comments,
                                      'InputPos' : inputPos,
                                      'OutputPos' : outputPos
                                  })
                    }
                }

                MergeReportView {
                    id: mergeReportView
                    anchors.fill: parent
                    colors: root.colors

                    function refresh() {
                        dataModel.clear();

                        for(var i = 0; i < csvParser.rows.length; ++i) {
                            var used = csvParser.rows[i][fileSelectionColumn];
                            if(i === 0 || i === (csvParser.rows.length - 1) || used !== '0')
                                dataModel.appendRow(csvParser.rows[i])
                        }
                    }

                    onFileSelectionClicked: {
                        mergeAnalyzeView.index = row;
                        mergeAnalyzeView.fetch();
                        mergeAnalyzeView.open();
                    }

                    Component.onCompleted: {

                    }
                }
            }
        }
    }
}
