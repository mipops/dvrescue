import QtQuick 2.0
import QtQuick 2.12
import Qt.labs.qmlmodels 1.0
import SortFilterTableModel 1.0

Rectangle {
    property alias model: dataModel
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
            width: tableView.view.columnWidthProvider(modelData)
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
            color: (row % 2) == 0 ? evenColor : oddColor
        }

        function getTotalDesiredWidth() {
            var value = 0
            for(var i = 0; i < tableView.model.columnCount; ++i)
            {
                var headerItem = tableView.getHeaderItem(i)
                if(headerItem === null)
                    continue;

                // console.debug('headerItem: ', headerItem)
                var minWidth = dataModel.columns[i].minWidth
                value += Math.max(headerItem.desiredWidth, minWidth)
            }

            // console.debug('new totalDesiredWidth: ', value)
            return value;
        }

        property bool initialized: false
        property int totalDesiredWidth: {
            return tableView.width, tableView.model.columnsCount, initialized, getTotalDesiredWidth()
        }

        columnWidthProvider: function(column) {
            var minWidth = dataModel.columns[column].minWidth
            var desiredWidth = Math.max(tableView.getHeaderItem(column).desiredWidth, minWidth)

            var relativeWidth = tableView.totalDesiredWidth !== 0 ? (desiredWidth / tableView.totalDesiredWidth) : 0
            var allowedWidth = relativeWidth * tableView.width

            return Math.max(allowedWidth, desiredWidth);
        }

        Component.onCompleted: {
            Qt.callLater(() => {
                             initialized = true
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

    }
}
