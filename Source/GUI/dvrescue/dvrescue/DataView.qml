import QtQuick 2.0
import QtQuick 2.12
import TableModel 1.0
import TableModelColumn 1.0
import SortFilterTableModel 1.0

Rectangle {
    id: dataView
    property alias model: dataModel
    property var cppDataModel;
    property int framePos: -1

    signal tapped(int framePos);

    function bringToView(framePos) {
        var sourceRow = cppDataModel.rowByFrame(framePos);
        if(sourceRow !== -1)
        {
            var row = sortFilterTableModel.fromSourceRowIndex(sourceRow)
            Qt.callLater(() => {
                             tableView.bringToView(row)
                         })
        }
    }

    onWidthChanged: {
        tableView.forceLayout();
    }

    TableViewEx {
        id: tableView
        anchors.fill: parent
        anchors.margins: 10
        model: sortFilterTableModel
        delegateHeight: 35

        headerDelegate: SortableFiltrableColumnHeading {
            width: tableView.columnWidths[modelData]
            text: dataModel.columns[modelData].display
            canFilter: true
            canSort: false

            onFilterTextChanged: {
                sortFilterTableModel.setFilterText(modelData, filterText);
            }

            height: tableView.delegateHeight * 2.5
        }

        delegate: TextDelegate {
            height: tableView.delegateHeight
            implicitHeight: tableView.delegateHeight
            property color evenColor: '#e3e3e3'
            property color oddColor: '#f3f3f3'
            property color redColor: 'red'
            color: {

                var sourceRow = sortFilterTableModel.toSourceRowIndex(row);
                var frameNumber = cppDataModel.frameByIndex(sourceRow);
                // var frameNumber = dataModel.getRow(sourceRow)[0]; // slow approach
                if(frameNumber === framePos) {
                    return redColor;
                }

                return (row % 2) == 0 ? evenColor : oddColor
            }

            TapHandler {
                onTapped: {
                    var sourceRow = sortFilterTableModel.toSourceRowIndex(row);
                    var frameNumber = cppDataModel.frameByIndex(sourceRow);
                    dataView.tapped(frameNumber);
                }
            }
        }

        function getTotalDesiredWidth() {
            var value = 0
            for(var i = 0; i < tableView.model.columnCount; ++i)
            {
                var headerItem = tableView.getHeaderItem(i)
                if(headerItem === null) {
                    console.debug('headerItem === null')
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
            var minWidth = dataModel.columns[column].minWidth
            var desiredWidth = Math.max(tableView.getHeaderItem(column).desiredWidth, minWidth)

            var relativeWidth = tableView.totalDesiredWidth !== 0 ? (desiredWidth / tableView.totalDesiredWidth) : 0
            var allowedWidth = relativeWidth * tableView.width

            var columnWidth = Math.max(allowedWidth, desiredWidth);
            return columnWidth
        }

        property bool initialized: false
        property int totalDesiredWidth: {
            return initialized, getTotalDesiredWidth()
        }

        function updateColumnWidths() {
            var newColumnWidths = {}
            var totalWidth = 0;
            for(var i = 0; i < tableView.model.columnCount; ++i)
            {
                newColumnWidths[i] = getColumnWidth(i)
                console.debug('column', i, 'width: ', newColumnWidths[i])
                totalWidth += newColumnWidths[i]
            }

            totalWidth += (tableView.model.columnCount - 1) * tableView.view.columnSpacing
            columnWidths = newColumnWidths

            tableView.view.contentWidth = totalWidth
            console.debug('tableView.view.contentWidth: ', tableView.view.contentWidth)
        }

        property var columnWidths: ({})
        onWidthChanged: {
            updateColumnWidths()
        }

        columnWidthProvider: function(column) {
            return tableView.columnWidths[column];
        }

        Component.onCompleted: {
            Qt.callLater(() => {
                             initialized = true;
                             updateColumnWidths();
                         })
        }
    }

    SortFilterTableModel {
        id: sortFilterTableModel
        tableModel: dataModel
    }

    TextMetrics {
        id: timestampMetrics
        text: "00:00:00.000000"
    }
    TextMetrics {
        id: timecodeMetrics
        text: "00:00:00:00"
    }
    TextMetrics {
        id: recordingTimeMetrics
        text: "0000-00-00 00:00:00"
    }

    property int columnSpacing: 10

    TableModel {
        id: dataModel

        TableModelColumn {
            display: "Frame #";
            property int minWidth: 20
        }

        TableModelColumn {
            display: "Byte Offset";
            property int minWidth: 20
        }

        TableModelColumn {
            display: "Timestamp";
            property int minWidth: timestampMetrics.width + columnSpacing
        }

        TableModelColumn {
            display: "Timecode";
            property int minWidth: timecodeMetrics.width + columnSpacing
        }

        TableModelColumn {
            display: "Timecode Repeat";
            property int minWidth: 20
        }

        TableModelColumn {
            display: "Timecode Jump";
            property int minWidth: 20
        }

        TableModelColumn {
            display: "Recording Time"
            property int minWidth: recordingTimeMetrics.width + columnSpacing
        }

        TableModelColumn {
            display: "Recording Time Repeat";
            property int minWidth: 20
        }

        TableModelColumn {
            display: "Recording Time Jump";
            property int minWidth: 20
        }

        TableModelColumn {
            display: "Recording Start";
            property int minWidth: 20
        }

        TableModelColumn {
            display: "Recording End";
            property int minWidth: 20
        }

        TableModelColumn {
            display: "Arbitrary Bits";
            property int minWidth: 20
        }

        TableModelColumn {
            display: "Arbitrary Bits Repeat";
            property int minWidth: 20
        }

        TableModelColumn {
            display: "Arbitrary Bits Jump";
            property int minWidth: 20
        }

        TableModelColumn {
            display: "Captions";
            property int minWidth: 20
        }

        TableModelColumn {
            display: "Caption Parity";
            property int minWidth: 20
        }

        TableModelColumn {
            display: "No Pack";
            property int minWidth: 20
        }

        TableModelColumn {
            display: "No Subcode Pack";
            property int minWidth: 20
        }

        TableModelColumn {
            display: "No Video Pack";
            property int minWidth: 20
        }

        TableModelColumn {
            display: "No Audio Pack";
            property int minWidth: 20
        }

        TableModelColumn {
            display: "No Video Source or Control";
            property int minWidth: 20
        }

        TableModelColumn {
            display: "No Audio Source or Control";
            property int minWidth: 20
        }

        TableModelColumn {
            display: "Full Conceal";
            property int minWidth: 20
        }

        TableModelColumn {
            display: "Full Conceal Video";
            property int minWidth: 20
        }

        TableModelColumn {
            display: "Full Conceal Audio";
            property int minWidth: 20
        }

        TableModelColumn {
            display: "Video Size";
            property int minWidth: 20
        }

        TableModelColumn {
            display: "Video Rate";
            property int minWidth: 20
        }

        TableModelColumn {
            display: "Chroma Subsampling";
            property int minWidth: 20
        }

        TableModelColumn {
            display: "Aspect Ratio";
            property int minWidth: 20
        }

        TableModelColumn {
            display: "Audio Rate";
            property int minWidth: 20
        }

        TableModelColumn {
            display: "Channels";
            property int minWidth: 20
        }

        TableModelColumn {
            display: "Video Error Concealment %";
            property int minWidth: 20
        }

        TableModelColumn {
            display: "Video Error Concealment % (odd/even balance)";
            property int minWidth: 20
        }

        TableModelColumn {
            display: "Audio Error %";
            property int minWidth: 20
        }

        TableModelColumn {
            display: "Audio Error % (odd/even balance)";
            property int minWidth: 20
        }

    }
}
