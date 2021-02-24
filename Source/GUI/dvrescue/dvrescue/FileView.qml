import QtQuick 2.0
import QtQuick 2.12
import TableModel 1.0
import TableModelColumn 1.0
import SortFilterTableModel 1.0
import QtQuick.Controls 2.12
import QtQml 2.12
import Qt.labs.qmlmodels 1.0
import MediaInfo 1.0

Rectangle {    
    property int updated: 0
    property var fileInfos: []
    property var files: []
    property alias tableView: tableView
    readonly property string selectedPath: tableView.currentIndex !== -1 ? fileInfos[tableView.currentIndex].originalPath : ''
    property alias currentIndex: tableView.currentIndex

    function add(path) {
        files.push(path);
        var mediaInfo = report.resolveRelatedInfo(path);
        fileInfos.push(mediaInfo)
        ++updated
        newRow(mediaInfo.originalPath)
    }

    readonly property string fileNameColumn: "File Name"
    readonly property string formatColumn: "Format"
    readonly property string fileSizeColumn: "File Size"
    readonly property string frameCountColumn: "Frame Count"
    readonly property string countOfFrameSequencesColumn: "Count of Frame Sequences"
    readonly property string firstTimecodeColumn: "First Timecode"
    readonly property string lastTimecodeColumn: "Last Timecode"
    readonly property string firstRecordingTimeColumn: "First Recording Time"
    readonly property string lastRecordingTimeColumn: "Last Recording Time"

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
        model: updated, fileInfos.length
        onModelChanged: {
            console.debug('model: ', model)
        }
        delegate: MediaInfo {
            reportPath: index >= 0 ? fileInfos[index].reportPath : ''
            videoPath: index >= 0 ? fileInfos[index].videoPath : ''

            function editRow(index, propertyName, propertyValue) {
                var rowData = dataModel.getRow(index)
                var newRowData = JSON.parse(JSON.stringify(rowData))
                newRowData[propertyName] = propertyValue
                dataModel.setRow(index, newRowData)
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
                editRow(index, "Progress", parsing === false ? 1 : 0)
            }
            onBytesProcessedChanged: {
                editRow(index, "Progress", bytesProcessed / reportFileSize)
                console.debug('bytesProcessed / reportFileSize: ', bytesProcessed / reportFileSize)
            }

            Component.onCompleted: {
                resolve();
            }
        }
    }

    function newRow(path) {
        var rowEntry = {}
        rowEntry[fileNameColumn] = path
        rowEntry[formatColumn] = " "
        rowEntry[fileSizeColumn] = " "
        rowEntry[frameCountColumn] = " "
        rowEntry[countOfFrameSequencesColumn] = " "
        rowEntry[firstTimecodeColumn] = " "
        rowEntry[lastTimecodeColumn] = " "
        rowEntry[firstRecordingTimeColumn] = " "
        rowEntry[lastRecordingTimeColumn] = " "
        rowEntry["Progress"] = 0;

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

        headerDelegate: SortableFiltrableColumnHeading {
            id: header
            width: tableView.columnWidths[modelData]
            text: dataModel.columns[modelData].display
            canFilter: false
            canSort: false
            filterFont.pixelSize: 11
            textFont.pixelSize: 13
            height: tableView.getMaxDesiredHeight()

            onFilterTextChanged: {
                sortFilterTableModel.setFilterText(modelData, filterText);
            }

            Rectangle {
                id: handle
                color: Qt.darker(parent.color, 1.05)
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

                    overlayColor: row == tableView.currentIndex ? 'green' : 'lightgray'
                    overlayVisible: decoration !== 1 || row == tableView.currentIndex
                    progress.visible: decoration !== 1
                    progress.value: decoration
                    progressColor: 'gray'
                    color: (row % 2) == 0 ? evenColor : oddColor
                    MouseArea {
                        anchors.fill: parent
                        onDoubleClicked: {
                            tableView.currentIndex = row
                        }
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
                        anchors.fill: parent
                        onDoubleClicked: {
                            tableView.currentIndex = row
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

            console.debug('new totalDesiredWidth: ', value, 'tableView.width: ', tableView.width)
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

    TableModel {
        id: dataModel

        property var columnsNames: {
            var names = [];
            for(var i = 0; i < columns.length; ++i) {
                names.push(columns[i].display)
            }
            return names;
        }

        TableModelColumn {
            display: "File Name"
            decoration: "Progress"
            property int minWidth: 200
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
    }
}
