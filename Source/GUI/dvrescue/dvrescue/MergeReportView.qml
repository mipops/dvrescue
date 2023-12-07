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
    id: root
    color: 'transparent'

    signal fileSelectionClicked(var row);

    property alias dataModel: dataModel
    property alias tableView: tableView

    readonly property string framePosColumn: "Frame #"
    readonly property string timeCodeColumn: "Timecode"
    readonly property string fileSelectionColumn: "File Selection"
    readonly property string statusColumn: "Status"
    readonly property string issueFixedColumn: "IssueFixed"
    readonly property string blockErrorsColumn: "BlockErrors"
    readonly property string commentsColumn: "Comments"

    property var inputFiles: []
    property var outputFile;

    property var colors: []

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

            property int blockErrorsColumnIndex: {
                console.debug('blockErrorsColumnIndex: dataModel.columnsNames: ', JSON.stringify(dataModel.columnsNames))
                return dataModel.columnsNames.indexOf(blockErrorsColumn);
            }
            onBlockErrorsColumnIndexChanged: {
                console.debug('blockErrorsColumnIndex: ', blockErrorsColumnIndex)
            }

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
                    column: 2
                    HyperlinkDelegate {
                        height: tableView.delegateHeight
                        implicitHeight: tableView.delegateHeight
                        property color evenColor: '#e3e3e3'
                        property color oddColor: '#f3f3f3'
                        property color redColor: 'red'
                        textFont.pixelSize: 13
                        text: '<a href="#">' + display + '</a>'
                        overlayVisible: true
                        overlayColor: foreground < colors.length ? colors[foreground] : 'transparent'
                        onLinkActivated: {
                            var sourceRow = sortFilterTableModel.toSourceRowIndex(row);
                            root.fileSelectionClicked(sourceRow);
                        }

                        color: (row % 2) == 0 ? evenColor : oddColor
                    }
                }

                DelegateChoice  {
                    column: 3
                    StatusDelegate {
                        height: tableView.delegateHeight
                        implicitHeight: tableView.delegateHeight
                        property color evenColor: '#e3e3e3'
                        property color oddColor: '#f3f3f3'
                        overlayVisible: true
                        overlayColor: foreground < colors.length ? colors[foreground] : 'transparent'

                        status: display.padEnd(inputFiles.length);
                        color: (row % 2) == 0 ? evenColor : oddColor

                        MouseArea {
                            id: statusMouseArea
                            hoverEnabled: true
                            anchors.fill: parent
                        }

                        DefaultToolTip {
                            visible: statusMouseArea.containsMouse
                            text: {
                                var rowData = dataModel.getRow(row);

                                var text = 'For ${timecode}: '.replace('${timecode}', rowData[timeCodeColumn]);
                                var status = rowData[statusColumn].padEnd(inputFiles.length)

                                for(var i = 0; i < status.length; ++i) {
                                    text += 'Input {number} ({status}); '.replace('{number}', i).replace('{status}', status[i] === 'M' ? 'missing' : status[i] === 'P' ? 'problem' : 'ok')
                                }

                                return text;
                            }
                            delay: 1500
                            anchors.centerIn: parent
                        }
                    }

                    /*
                    TextDelegate {
                        height: tableView.delegateHeight
                        implicitHeight: tableView.delegateHeight
                        property color evenColor: '#e3e3e3'
                        property color oddColor: '#f3f3f3'
                        property color redColor: 'red'
                        textFont.pixelSize: 13
                        text: display
                        overlayVisible: true
                        overlayColor: foreground < colors.length ? colors[foreground] : 'transparent'

                        color: (row % 2) == 0 ? evenColor : oddColor
                    }
                    */
                }

                DelegateChoice  {
                    column: 5
                    OddEvenTextDelegate {
                        height: tableView.delegateHeight
                        implicitHeight: tableView.delegateHeight
                        property color evenColor: '#e3e3e3'
                        property color oddColor: '#f3f3f3'
                        property color redColor: 'red'
                        textFont.pixelSize: 13
                        text: display
                        overlayVisible: true
                        overlayColor: foreground < colors.length ? colors[foreground] : 'transparent'

                        evenProgressColor: edit ? 'yellow' : 'darkgreen'
                        oddProgressColor: edit ? 'yellow' : 'green'
                        evenProgress.value: edit ? 1 : decoration.x
                        oddProgress.value: edit ? 1 : decoration.y

                        color: (row % 2) == 0 ? evenColor : oddColor
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
                        overlayVisible: true
                        overlayColor: foreground < colors.length ? colors[foreground] : 'transparent'

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
            foreground: fileSelectionColumn
            property int minWidth: 20
        }

        TableModelColumn {
            display: timeCodeColumn
            foreground: fileSelectionColumn
            property int minWidth: 50
        }

        TableModelColumn {
            display: fileSelectionColumn
            foreground: fileSelectionColumn
            property int minWidth: 50
        }

        TableModelColumn {
            display: statusColumn
            foreground: fileSelectionColumn
            property int minWidth: 50 * inputFiles.length
        }

        TableModelColumn {
            display: issueFixedColumn
            foreground: fileSelectionColumn
            property int minWidth: 50
        }

        TableModelColumn {
            display: blockErrorsColumn
            foreground: fileSelectionColumn

            decoration: "blockErrorsEvenOdd";
            edit: "hasNoBlockErrors";

            property int minWidth: 50
        }
    }
}
