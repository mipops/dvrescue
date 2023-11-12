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
    color: 'transparent'

    property alias dataModel: dataModel
    property alias tableView: tableView

    readonly property string framePosColumn: "Frame #"
    readonly property string timeCodeColumn: "Timecode"
    readonly property string fileSelectionColumn: "File Selection"
    readonly property string statusColumn: "Status"
    readonly property string issueFixedColumn: "IssueFixed"
    readonly property string blockErrorsColumn: "BlockErrors"
    readonly property string commentsColumn: "Comments"

    function forceLayout() {
        tableView.forceLayout();
    }

    onWidthChanged: {
        tableView.forceLayout();
    }
    onHeightChanged: {
        tableView.forceLayout();
    }

    function newRow() {
        var rowEntry = {}

        rowEntry[framePosColumn] = ''
        rowEntry[timeCodeColumn] = ''
        rowEntry[fileSelectionColumn] = ''
        rowEntry[statusColumn] = ''
        rowEntry[issueFixedColumn] = ''
        rowEntry[blockErrorsColumn] = ''
        rowEntry[commentsColumn] = ''

        dataModel.appendRow(rowEntry)
    }

    function scrollToTop() {
        tableView.view.contentY = -16
    }

    Text {
        id: label
        text: 'SUMMARY'
        color: 'white'
        font.pixelSize: 20
    }

    Rectangle {
        color: 'white'
        anchors.top: label.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        TableViewEx {
            id: tableView
            anchors.fill: parent
            anchors.margins: 1
            model: sortFilterTableModel
            dataModel: dataModel
            delegateHeight: 25

            headerDelegate: SortableFiltrableColumnHeading {
                id: header
                width: tableView.columnWidths[modelData] ? tableView.columnWidths[modelData] : 50
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
                    TextDelegate {
                        height: tableView.delegateHeight
                        implicitHeight: tableView.delegateHeight
                        property color evenColor: '#e3e3e3'
                        property color oddColor: '#f3f3f3'
                        property color redColor: 'red'
                        textFont.pixelSize: 13
                        text: display

                        color: (row % 2) == 0 ? evenColor : oddColor
                    }
                }
            }
        }
    }

    SortFilterTableModel {
        id: sortFilterTableModel
        tableModel: dataModel
    }

    property int columnSpacing: 10

    TableModelEx {
        id: dataModel

        TableModelColumn {
            display: framePosColumn
            property int minWidth: 20
        }

        TableModelColumn {
            display: timeCodeColumn
            property int minWidth: 50
        }

        TableModelColumn {
            display: fileSelectionColumn
            property int minWidth: 50
        }

        TableModelColumn {
            display: statusColumn
            property int minWidth: 50
        }

        TableModelColumn {
            display: issueFixedColumn
            property int minWidth: 50
        }

        TableModelColumn {
            display: blockErrorsColumn
            property int minWidth: 50
        }

        TableModelColumn {
            display: commentsColumn
            property int minWidth: 100
        }
    }
}
