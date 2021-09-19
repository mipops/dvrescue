import QtQuick 2.0
import QtQuick 2.12
import TableModel 1.0
import TableModelColumn 1.0
import SortFilterTableModel 1.0
import QtQuick.Controls 2.12
import QtQml 2.12
import Qt.labs.qmlmodels 1.0
import MediaInfo 1.0
import FileUtils 1.0

Rectangle {
    signal fileAdded(string filePath);

    property int updated: 0
    property var fileInfos: []
    property var files: []
    property alias tableView: tableView
    readonly property string selectedPath: {
        console.debug('selected path changed: ', tableView.currentIndex)
        return updated, tableView.currentIndex !== -1 ? fileInfos[tableView.currentIndex].originalPath : ''
    }
    property alias currentIndex: tableView.currentIndex
    onCurrentIndexChanged: {
        console.debug('AnalyseFileView: currentIndex = ', currentIndex)
    }

    function add(path) {
        console.debug('add: ', path, 'currentIndex: ', currentIndex);

        files.push(path);
        var mediaInfo = report.resolveRelatedInfo(path);
        fileInfos.push(mediaInfo)
        newRow(mediaInfo.originalPath)
        fileAdded(mediaInfo.originalPath)

        if(fileInfos.length === 1)
            tableView.bringToView(0)

        ++updated
    }

    function deleteEntry(row) {
        console.debug('deleteEntry: ', row);

        files.splice(row,  1);
        fileInfos.splice(row, 1);
        dataModel.removeRow(row, 1);

        ++updated
    }

    function mediaInfoAt(index) {
        return instantiator.objectAt(index);
    }

    readonly property string filePathColumn: "File Path"
    readonly property string fileNameColumn: "File Name"
    readonly property string formatColumn: "Format"
    readonly property string fileSizeColumn: "File Size"
    readonly property string frameCountColumn: "Frame Count"
    readonly property string countOfFrameSequencesColumn: "Count of Frame Sequences"
    readonly property string firstTimecodeColumn: "First Timecode"
    readonly property string lastTimecodeColumn: "Last Timecode"
    readonly property string firstRecordingTimeColumn: "First Recording Time"
    readonly property string lastRecordingTimeColumn: "Last Recording Time"
    readonly property string frameErrorColumn: "Frame Error %"
    readonly property string videoBlockErrorColumn: "Video Block Error %"
    readonly property string audioBlockErrorColumn: "Audio Block Error %"

    readonly property string progressRole: "Progress"
    readonly property string frameErrorTooltipRole: "Frame Error Tooltip"
    readonly property string videoBlockErrorTooltipRole: "Video Block Error Tooltip"
    readonly property string audioBlockErrorTooltipRole: "Audio Block Error Tooltip"
    readonly property string videoBlockErrorValueRole: "Video Block Error Value"
    readonly property string audioBlockErrorValueRole: "Audio Block Error Value"

    DvRescueReport {
        id: report
    }

    function forceLayout() {
        tableView.forceLayout();
    }

    onWidthChanged: {
        tableView.forceLayout();
    }
    onHeightChanged: {
        tableView.forceLayout();
    }

    Instantiator {
        id: instantiator
        model: updated, fileInfos.length
        onModelChanged: {
            console.debug('model: ', model)
        }
        delegate: MediaInfo {
            reportPath: {
                console.debug('reportPath: ', index, fileInfos.length)
                return (index >= 0 && index < fileInfos.length) ? fileInfos[index].reportPath : ''
            }
            videoPath: {
                console.debug('videoPath: ', index, fileInfos.length)
                return (index >= 0 && index < fileInfos.length) ? fileInfos[index].videoPath : ''
            }

            function editRow(index, propertyName, propertyValue) {
                // console.debug('key: ', propertyName, 'value: ', JSON.stringify(propertyValue))
                if(index >= 0 && index < fileInfos.length)
                {
                    var rowData = dataModel.getRow(index)
                    var newRowData = JSON.parse(JSON.stringify(rowData))
                    newRowData[propertyName] = propertyValue
                    dataModel.setRow(index, newRowData)
                }
            }

            onFormatChanged: {
                editRow(index, formatColumn, format)
            }
            onFileSizeChanged: {
                editRow(index, fileSizeColumn, fileSize)
            }
            onFrameCountChanged: {
                editRow(index, frameCountColumn, frameCount)
            }
            onCountOfFrameSequencesChanged: {
                editRow(index, countOfFrameSequencesColumn, countOfFrameSequences)
            }
            onFirstTimeCodeChanged: {
                editRow(index, firstTimecodeColumn, firstTimeCode)
            }
            onLastTimeCodeChanged: {
                editRow(index, lastTimecodeColumn, lastTimeCode)
            }
            onFirstRecordingTimeChanged: {
                editRow(index, firstRecordingTimeColumn, firstRecordingTime)
            }
            onLastRecordingTimeChanged: {
                editRow(index, lastRecordingTimeColumn, lastRecordingTime)
            }
            onParsingChanged: {
                editRow(index, progressRole, parsing === false ? 1 : 0)
            }
            onBytesProcessedChanged: {
                editRow(index, progressRole, bytesProcessed / reportFileSize)
            }
            onFrameErrorChanged: {
                editRow(index, frameErrorColumn, frameError);
                editRow(index, frameErrorTooltipRole, "Sta count: " + staCount + ", Frames count: " + frameCount)
            }
            onVideoBlockErrorChanged: {
                editRow(index, videoBlockErrorColumn, videoBlockError);
                editRow(index, videoBlockErrorTooltipRole, "Sta sum: " + staSum + ", Sum of video blocks: " + totalVideoBlocks)
                editRow(index, videoBlockErrorValueRole, { x : videoBlockErrorValue.x, y : videoBlockErrorValue.y })
            }
            onAudioBlockErrorChanged: {
                editRow(index, audioBlockErrorColumn, audioBlockError);
                editRow(index, audioBlockErrorTooltipRole, "Aud sum: " + audSum + ", Sum of audio blocks: " + totalAudioBlocks)
                editRow(index, audioBlockErrorValueRole, { x : audioBlockErrorValue.x, y : audioBlockErrorValue.y })
            }

            Component.onCompleted: {
                resolve();
            }
        }
    }

    function newRow(path) {
        var rowEntry = {}
        rowEntry[filePathColumn] = path
        rowEntry[fileNameColumn] = FileUtils.getFileName(path)
        rowEntry[formatColumn] = " "
        rowEntry[fileSizeColumn] = " "
        rowEntry[frameCountColumn] = " "
        rowEntry[countOfFrameSequencesColumn] = " "
        rowEntry[firstTimecodeColumn] = " "
        rowEntry[lastTimecodeColumn] = " "
        rowEntry[firstRecordingTimeColumn] = " "
        rowEntry[lastRecordingTimeColumn] = " "
        rowEntry[frameErrorColumn] = " "
        rowEntry[videoBlockErrorColumn] = " "
        rowEntry[audioBlockErrorColumn] = " "

        rowEntry[progressRole] = 0
        rowEntry[frameErrorTooltipRole] = " "
        rowEntry[videoBlockErrorTooltipRole] = " "
        rowEntry[audioBlockErrorTooltipRole] = " "
        rowEntry[videoBlockErrorValueRole] = { x : 0, y : 0 }
        rowEntry[audioBlockErrorValueRole] = { x : 0, y : 0 }

        dataModel.appendRow(rowEntry)
    }

    function scrollToTop() {
        tableView.view.contentY = -16
    }

    onFilesChanged: {
        var newFileInfos = [];

       Qt.callLater(() => {
                         if(files) {
                             files.forEach((file) => {
                                 var mediaInfo = report.resolveRelatedInfo(file)
                                 newFileInfos.push(mediaInfo)
                                 newRow(mediaInfo.originalPath)
                             })

                            console.debug('tableView.view.contentY: ', tableView.view.contentY)
                            scrollToTop();
                         }

                        fileInfos = newFileInfos;
                     })
    }

    TableViewEx {
        id: tableView
        anchors.fill: parent
        anchors.margins: 10
        model: sortFilterTableModel
        delegateHeight: 25
        property int currentIndex: -1

        Menu {
            id: contextMenu
            MenuItem {
                text: "Delete"
                onClicked: deleteEntry(currentIndex)
            }

            function showBelowControl(control) {
                var mapped = control.mapToItem(tableView, 0, 0);
                contextMenu.x = mapped.x
                contextMenu.y = mapped.y + control.height

                contextMenu.open()
            }

            function show(pos) {
                contextMenu.x = pos.x
                contextMenu.y = pos.y

                contextMenu.open()
            }
        }

        headerDelegate: SortableFiltrableColumnHeading {
            id: header
            width: tableView.columnWidths[modelData]
            text: dataModel.columns[modelData].display
            canFilter: false
            canSort: false
            canShowIndicator: false
            filterFont.pixelSize: 11
            textFont.pixelSize: 13
            height: tableView.columnWidths, tableView.getMaxDesiredHeight()

            onFilterTextChanged: {
                sortFilterTableModel.setFilterText(modelData, filterText);
            }

            Rectangle {
                id: handle
                color: "transparent"
                height: parent.height
                width: 10
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                MouseArea {
                    id: mouseHandle
                    anchors.fill: parent
                    drag{ target: parent; axis: Drag.XAxis }
                    hoverEnabled: true
                    cursorShape: Qt.SizeHorCursor
                    onMouseXChanged: {
                        if (drag.active) {
                            var newWidth = header.width + mouseX
                            if (newWidth >= minimumWidth) {
                                // header.width = newWidth

                                var newWidths = tableView.columnWidths
                                var oldWidth = newWidths[modelData]
                                newWidths[modelData] = newWidth;

                                tableView.columnWidths = newWidths
                                tableView.view.contentWidth += newWidth - oldWidth
                                tableView.forceLayout();
                            }
                        }
                    }
                }
            }
        }

        delegate: DelegateChooser {
            DelegateChoice  {
                column: 0

                ProgressTextDelegate {
                    height: tableView.delegateHeight
                    implicitHeight: tableView.delegateHeight
                    property color evenColor: '#e3e3e3'
                    property color oddColor: '#f3f3f3'
                    property color redColor: 'red'
                    textFont.pixelSize: 13
                    text: display
                    leftOffset: deleteButton.width + 4

                    overlayColor: row == tableView.currentIndex ? 'green' : 'lightgray'
                    overlayVisible: decoration !== 1 || row == tableView.currentIndex
                    progress.visible: decoration !== 1
                    progress.value: decoration
                    progressColor: 'gray'
                    color: (row % 2) == 0 ? evenColor : oddColor
                    MouseArea {
                        id: filePathMouseArea
                        acceptedButtons: Qt.LeftButton | Qt.RightButton
                        hoverEnabled: true
                        anchors.fill: parent
                        onDoubleClicked: {
                            tableView.currentIndex = row
                        }
                        onClicked: {
                            if(mouse.button == Qt.RightButton) {
                                contextMenu.show(mapToItem(tableView, mouseX, mouseY));
                            }
                        }
                    }

                    DefaultToolTip {
                        visible: filePathMouseArea.containsMouse
                        text: toolTip
                        anchors.centerIn: parent
                    }

                    CustomButton {
                        id: deleteButton
                        anchors.left: parent.left
                        anchors.leftMargin: 2

                        icon.color: "black"
                        icon.source: "icons/exit.svg"
                        implicitHeight: parent.height
                        implicitWidth: implicitHeight

                        onClicked: {
                            deleteEntry(row);
                        }
                    }

                }
            }

            DelegateChoice  {
                column: 8

                TextDelegate {
                    height: tableView.delegateHeight
                    implicitHeight: tableView.delegateHeight
                    property color evenColor: '#e3e3e3'
                    property color oddColor: '#f3f3f3'
                    property color redColor: 'red'
                    textFont.pixelSize: 13
                    text: display

                    overlayColor: row == tableView.currentIndex ? 'green' : 'lightgray'
                    overlayVisible: decoration !== 1 || row == tableView.currentIndex
                    color: (row % 2) == 0 ? evenColor : oddColor

                    MouseArea {
                        acceptedButtons: Qt.LeftButton | Qt.RightButton
                        anchors.fill: parent
                        onDoubleClicked: {
                            tableView.currentIndex = row
                        }
                        onClicked: {
                            if(mouse.button == Qt.RightButton) {
                                contextMenu.show(mapToItem(tableView, mouseX, mouseY));
                            }
                        }
                    }
                }
            }

            DelegateChoice  {
                column: 9

                TextDelegate {
                    height: tableView.delegateHeight
                    implicitHeight: tableView.delegateHeight
                    property color evenColor: '#e3e3e3'
                    property color oddColor: '#f3f3f3'
                    property color redColor: 'red'
                    textFont.pixelSize: 13
                    text: display

                    overlayColor: row == tableView.currentIndex ? 'green' : 'lightgray'
                    overlayVisible: decoration !== 1 || row == tableView.currentIndex
                    color: (row % 2) == 0 ? evenColor : oddColor
                    MouseArea {
                        id: frameErrorMouseArea
                        acceptedButtons: Qt.LeftButton | Qt.RightButton
                        hoverEnabled: true
                        anchors.fill: parent
                        onDoubleClicked: {
                            tableView.currentIndex = row
                        }
                        onClicked: {
                            if(mouse.button == Qt.RightButton) {
                                contextMenu.show(mapToItem(tableView, mouseX, mouseY));
                            }
                        }
                    }

                    DefaultToolTip {
                        visible: frameErrorMouseArea.containsMouse
                        text: toolTip
                        anchors.centerIn: parent
                    }
                }
            }

            DelegateChoice  {
                column: 10

                OddEvenTextDelegate {
                    height: tableView.delegateHeight
                    implicitHeight: tableView.delegateHeight
                    property color evenColor: '#e3e3e3'
                    property color oddColor: '#f3f3f3'
                    property color redColor: 'red'
                    textFont.pixelSize: 13
                    text: display

                    evenProgressColor: 'darkgreen'
                    oddProgressColor: 'green'
                    evenProgress.value: edit.x
                    oddProgress.value: edit.y

                    overlayColor: row == tableView.currentIndex ? 'green' : 'lightgray'
                    overlayVisible: decoration !== 1 || row == tableView.currentIndex
                    color: (row % 2) == 0 ? evenColor : oddColor
                    MouseArea {
                        id: videoBlockErrorMouseArea
                        acceptedButtons: Qt.LeftButton | Qt.RightButton
                        hoverEnabled: true
                        anchors.fill: parent
                        onDoubleClicked: {
                            tableView.currentIndex = row
                        }
                        onClicked: {
                            if(mouse.button == Qt.RightButton) {
                                contextMenu.show(mapToItem(tableView, mouseX, mouseY));
                            }
                        }
                    }

                    DefaultToolTip {
                        visible: videoBlockErrorMouseArea.containsMouse
                        text: toolTip
                        anchors.centerIn: parent
                    }
                }
            }

            DelegateChoice  {
                column: 11

                OddEvenTextDelegate {
                    height: tableView.delegateHeight
                    implicitHeight: tableView.delegateHeight
                    property color evenColor: '#e3e3e3'
                    property color oddColor: '#f3f3f3'
                    property color redColor: 'red'
                    textFont.pixelSize: 13
                    text: display

                    evenProgressColor: 'darkblue'
                    oddProgressColor: 'blue'
                    evenProgress.value: edit.x
                    oddProgress.value: edit.y

                    overlayColor: row == tableView.currentIndex ? 'green' : 'lightgray'
                    overlayVisible: decoration !== 1 || row == tableView.currentIndex
                    color: (row % 2) == 0 ? evenColor : oddColor
                    MouseArea {
                        id: audioBlockErrorMouseArea
                        acceptedButtons: Qt.LeftButton | Qt.RightButton
                        hoverEnabled: true
                        anchors.fill: parent
                        onDoubleClicked: {
                            tableView.currentIndex = row
                        }
                        onClicked: {
                            if(mouse.button == Qt.RightButton) {
                                contextMenu.show(mapToItem(tableView, mouseX, mouseY));
                            }
                        }
                    }

                    DefaultToolTip {
                        visible: audioBlockErrorMouseArea.containsMouse
                        text: toolTip
                        anchors.centerIn: parent
                    }
                }
            }

            DelegateChoice  {
                TextDelegate {
                    height: tableView.delegateHeight
                    implicitHeight: tableView.delegateHeight
                    property color evenColor: '#e3e3e3'
                    property color oddColor: '#f3f3f3'
                    property color redColor: 'red'
                    textFont.pixelSize: 13
                    text: display

                    overlayColor: row == tableView.currentIndex ? 'green' : 'lightgray'
                    overlayVisible: decoration !== 1 || row == tableView.currentIndex
                    color: (row % 2) == 0 ? evenColor : oddColor
                    MouseArea {
                        acceptedButtons: Qt.LeftButton | Qt.RightButton
                        anchors.fill: parent
                        onDoubleClicked: {
                            tableView.currentIndex = row
                        }
                        onClicked: {
                            if(mouse.button == Qt.RightButton) {
                                contextMenu.show(mapToItem(tableView, mouseX, mouseY));
                            }
                        }
                    }
                }
            }
        }

        function getMaxDesiredHeight() {
            var value = 0
            for(var i = 0; i < tableView.model.columnCount; ++i)
            {
                var headerItem = tableView.getHeaderItem(i)
                if(headerItem === null) {
                    continue;
                }

                value = Math.max(value, headerItem.desiredHeight)
            }
            return value;
        }

        function getTotalDesiredWidth() {
            var value = 0
            for(var i = 0; i < tableView.model.columnCount; ++i)
            {
                var headerItem = tableView.getHeaderItem(i)
                if(headerItem === null) {
                    continue;
                }

                // console.debug('headerItem: ', headerItem)
                var minWidth = dataModel.columns[i].minWidth
                value += Math.max(headerItem.desiredWidth, minWidth)
            }

            return value;
        }

        function getColumnWidth(column) {
            var headerItem = tableView.getHeaderItem(column);
            if(headerItem === null)
                return 0;

            var minWidth = dataModel.columns[column].minWidth
            var desiredWidth = Math.max(headerItem.desiredWidth, minWidth)

            var relativeWidth = tableView.totalDesiredWidth !== 0 ? (desiredWidth / tableView.totalDesiredWidth) : 0
            var allowedWidth = relativeWidth * tableView.width

            var columnWidth = Math.max(allowedWidth, desiredWidth);
            return columnWidth
        }

        property bool initialized: false
        property int totalDesiredWidth: {
            return initialized, getTotalDesiredWidth()
        }

        function adjustColumnWidths() {
            var newColumnWidths = {}
            var totalWidth = 0;
            for(var i = 0; i < tableView.model.columnCount; ++i)
            {
                newColumnWidths[i] = getColumnWidth(i)
                totalWidth += newColumnWidths[i]
            }

            totalWidth += (tableView.model.columnCount - 1) * tableView.view.columnSpacing
            columnWidths = newColumnWidths

            tableView.view.contentWidth = totalWidth
            console.debug('tableView.view.contentWidth: ', tableView.view.contentWidth)
        }

        property var columnWidths: ({})
        onWidthChanged: {
            adjustColumnWidths()
        }

        columnWidthProvider: function(column) {
            return tableView.columnWidths[column];
        }

        Component.onCompleted: {
            Qt.callLater(() => {
                             initialized = true;
                             adjustColumnWidths();
                         })
        }
    }

    SortFilterTableModel {
        id: sortFilterTableModel
        tableModel: dataModel
    }

    TextMetrics {
        id: timestampMetrics
        text: "00:00:00.000"
    }
    TextMetrics {
        id: timecodeMetrics
        text: "00:00:00:00"
    }
    TextMetrics {
        id: recordingTimeMetrics
        text: "0000-00-00 00:00:00"
    }
    TextMetrics {
        id: missingPacksMetrics
        text: "Subcode, Video, Audio"
    }

    TextMetrics {
        id: errorConcealmentMetrics
        text: "Video Error Concealment %"
    }

    property int columnSpacing: 10

    TableModelEx {
        id: dataModel

        TableModelColumn {
            display: "File Name"
            decoration: "Progress"
            toolTip: "File Path"
            property int minWidth: 250
        }

        TableModelColumn {
            display: "Format"
            decoration: "Progress"
            property int minWidth: 40
        }

        TableModelColumn {
            display: "File Size"
            decoration: "Progress"
            property int minWidth: 60
        }

        TableModelColumn {
            display: "Frame Count"
            decoration: "Progress"
            property int minWidth: 40
        }

        TableModelColumn {
            display: "Count of Frame Sequences"
            decoration: "Progress"
            property int minWidth: 40
        }

        TableModelColumn {
            display: "First Timecode"
            decoration: "Progress"
            property int minWidth: 40
        }

        TableModelColumn {
            display: "Last Timecode"
            decoration: "Progress"
            property int minWidth: 40
        }

        TableModelColumn {
            display: "First Recording Time"
            decoration: "Progress"
            property int minWidth: 40
        }

        TableModelColumn {
            display: "Last Recording Time"
            decoration: "Progress"
            property int minWidth: 40
        }

        TableModelColumn {
            display: "Frame Error %"
            decoration: "Progress"
            toolTip: "Frame Error Tooltip"
            property int minWidth: 40
        }

        TableModelColumn {
            display: "Video Block Error %"
            decoration: "Progress"
            edit: "Video Block Error Value"
            toolTip: "Video Block Error Tooltip"
            property int minWidth: 40
        }

        TableModelColumn {
            display: "Audio Block Error %"
            decoration: "Progress"
            edit: "Audio Block Error Value"
            toolTip: "Audio Block Error Tooltip"
            property int minWidth: 40
        }
    }
}
