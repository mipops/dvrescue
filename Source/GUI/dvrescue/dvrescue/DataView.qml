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
    }

    SortFilterTableModel {
        id: sortFilterTableModel
        tableModel: dataModel
    }

    TableModel {
        id: dataModel

        TableModelColumn {
            display: "Frame #";
        }

        TableModelColumn {
            display: "Byte Offset";
        }

        TableModelColumn {
            display: "Timestamp";
        }

        TableModelColumn {
            display: "Timecode";
        }

        TableModelColumn {
            display: "Timecode Repeat";
        }

        TableModelColumn {
            display: "Timecode Jump";
        }
    }
}
