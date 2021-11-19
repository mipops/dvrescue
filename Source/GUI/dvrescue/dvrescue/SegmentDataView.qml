import QtQuick 2.0
import QtQuick 2.12
import TableModel 1.0
import TableModelColumn 1.0
import SortFilterTableModel 1.0
import QtQuick.Controls 2.12
import Qt.labs.qmlmodels 1.0

Rectangle {
    id: dataView
    property int total: 0
    property alias model: dataModel
    property color rowHighlightColor: 'purple'
    property int currentIndex: -1
    property alias rowFilter: sortFilterTableModel.rowFilter
    property var hoveredItem: null
    function setHoveredItem(currentIndex, item) {
        var startFrame = item['Frame #']
        var endFrame = total - 1;
        if((currentIndex + 1) < model.rowCount) {
            endFrame = model.getRow(currentIndex + 1)['Frame #']
        }

        hoveredItem = { 'index' : currentIndex, 'range' : Qt.vector2d(startFrame, endFrame) }
    }

    signal clicked(var index, var item);
    signal doubleClicked(var index, var item);

    function invalidateFilter() {
        sortFilterTableModel.invalidateFilter();
    }

    onWidthChanged: {
        tableView.forceLayout();
    }

    TableViewEx {
        id: tableView
        anchors.fill: parent
        anchors.margins: 1
        model: sortFilterTableModel
        dataModel: dataModel
        delegateHeight: 25

        headerDelegate: SortableFiltrableColumnHeading {
            id: header
            width: tableView.columnWidths[modelData]
            text: dataModel.columns[modelData].display
            canFilter: false
            canSort: false
            canShowIndicator: false
            filterFont.pixelSize: 11
            textFont.pixelSize: 13
            height: tableView.getMaxDesiredHeight()

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
            DelegateChoice {
                column: dataModel.timecodeColumn

                JumpRepeatTextDelegate {
                    height: tableView.delegateHeight
                    implicitHeight: tableView.delegateHeight
                    property color evenColor: '#e3e3e3'
                    property color oddColor: '#f3f3f3'
                    textFont.pixelSize: 13
                    text: display
                    hasJump: decoration.x
                    hasRepeat: decoration.y

                    color: (row % 2) == 0 ? evenColor : oddColor
                    overlayColor: rowHighlightColor
                    overlayColorOpacity: row == currentIndex ? 0.5 : 0.25
                    overlayVisible: row == currentIndex || (hoveredItem !== null && row === hoveredItem.index)

                    MouseArea {
                        anchors.fill: parent
                        hoverEnabled: true
                        onEntered: {
                            var sourceRow = sortFilterTableModel.toSourceRowIndex(row);
                            var item = dataModel.getRow(sourceRow);
                            dataView.setHoveredItem(row, item);
                        }
                        onExited: {
                            dataView.hoveredItem = null
                        }

                        onClicked: {
                            var sourceRow = sortFilterTableModel.toSourceRowIndex(row);
                            var item = dataModel.getRow(sourceRow);

                            dataView.clicked(sourceRow, item);
                        }

                        onDoubleClicked: {
                            var sourceRow = sortFilterTableModel.toSourceRowIndex(row);
                            var item = dataModel.getRow(sourceRow);
                            dataView.doubleClicked(sourceRow, item);
                        }
                    }
                }
            }

            DelegateChoice {
                column: dataModel.recordingTimeColumn

                JumpRepeatTextDelegate {
                    height: tableView.delegateHeight
                    implicitHeight: tableView.delegateHeight
                    property color evenColor: '#e3e3e3'
                    property color oddColor: '#f3f3f3'
                    textFont.pixelSize: 13
                    text: display
                    hasJump: decoration.x
                    hasRepeat: decoration.y
                    property var editRole: edit

                    color: (row % 2) == 0 ? evenColor : oddColor
                    overlayColor: rowHighlightColor
                    overlayColorOpacity: row == currentIndex ? 0.5 : 0.25
                    overlayVisible: row == currentIndex || (hoveredItem !== null && row === hoveredItem.index)

                    Image {
                        id: image
                        height: parent.height
                        anchors.right: parent.right
                        fillMode: Image.PreserveAspectFit

                        property string imageUrl: {
                            if(editRole === Qt.point(1, 1)) {
                                return "icons/record-marker-stop+start-table.svg"
                            } else if(editRole === Qt.point(1, 0)) {
                                return "icons/record-marker-start-table.svg"
                            } else if(editRole === Qt.point(0, 1)) {
                                return "icons/record-marker-stop-table.svg"
                            }

                            return null;
                        }
                        visible: imageUrl !== null
                        Binding on source {
                            when: image.imageUrl != null
                            value: image.imageUrl
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        hoverEnabled: true
                        onEntered: {
                            var sourceRow = sortFilterTableModel.toSourceRowIndex(row);
                            var item = dataModel.getRow(sourceRow);
                            dataView.setHoveredItem(row, item);
                        }
                        onExited: {
                            dataView.hoveredItem = null
                        }

                        onClicked: {
                            var sourceRow = sortFilterTableModel.toSourceRowIndex(row);
                            var item = dataModel.getRow(sourceRow);
                            dataView.clicked(sourceRow, item);
                        }

                        onDoubleClicked: {
                            var sourceRow = sortFilterTableModel.toSourceRowIndex(row);
                            var item = dataModel.getRow(sourceRow);
                            dataView.doubleClicked(sourceRow, item);
                        }
                    }
                }
            }

            DelegateChoice  {
                column: dataModel.videoAudioColumn

                TextDelegate {
                    height: tableView.delegateHeight
                    implicitHeight: tableView.delegateHeight
                    property color evenColor: '#e3e3e3'
                    property color oddColor: '#f3f3f3'
                    textFont.pixelSize: 13
                    text: display

                    color: (row % 2) == 0 ? evenColor : oddColor
                    overlayColor: rowHighlightColor
                    overlayColorOpacity: row == currentIndex ? 0.5 : 0.25
                    overlayVisible: row == currentIndex || (hoveredItem !== null && row === hoveredItem.index)

                    MouseArea {
                        anchors.fill: parent
                        hoverEnabled: true;
                        onEntered: {
                            var sourceRow = sortFilterTableModel.toSourceRowIndex(row);
                            var item = dataModel.getRow(sourceRow);
                            dataView.setHoveredItem(row, item);
                        }
                        onExited: {
                            dataView.hoveredItem = null
                        }

                        onClicked: {
                            var sourceRow = sortFilterTableModel.toSourceRowIndex(row);
                            var item = dataModel.getRow(sourceRow);
                            dataView.clicked(sourceRow, item);
                        }

                        onDoubleClicked: {
                            var sourceRow = sortFilterTableModel.toSourceRowIndex(row);
                            var item = dataModel.getRow(sourceRow);
                            dataView.doubleClicked(sourceRow, item);
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
                    textFont.pixelSize: 13
                    text: display

                    color: (row % 2) == 0 ? evenColor : oddColor
                    overlayColor: rowHighlightColor
                    overlayColorOpacity: row == currentIndex ? 0.5 : 0.25
                    overlayVisible: row == currentIndex || (hoveredItem !== null && row === hoveredItem.index)

                    MouseArea {
                        anchors.fill: parent
                        hoverEnabled: true;
                        onEntered: {
                            var sourceRow = sortFilterTableModel.toSourceRowIndex(row);
                            var item = dataModel.getRow(sourceRow);
                            dataView.setHoveredItem(row, item);
                        }
                        onExited: {
                            dataView.hoveredItem = null
                        }

                        onClicked: {
                            var sourceRow = sortFilterTableModel.toSourceRowIndex(row);
                            var item = dataModel.getRow(sourceRow);
                            dataView.clicked(sourceRow, item);
                        }

                        onDoubleClicked: {
                            var sourceRow = sortFilterTableModel.toSourceRowIndex(row);
                            var item = dataModel.getRow(sourceRow);
                            dataView.doubleClicked(sourceRow, item);
                        }
                    }
                }
            }
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

    property int columnSpacing: 10

    TableModelEx {
        id: dataModel

        property int timestampColumn: columnsNames.indexOf("Timestamp");
        property int timecodeColumn: columnsNames.indexOf("Timecode");
        property int recordingTimeColumn: columnsNames.indexOf("Recording Time");
        property int videoAudioColumn: columnsNames.indexOf("Video/Audio");

        TableModelColumn {
            display: "Segment #";
            property int minWidth: 20
        }

        TableModelColumn {
            display: "Frame #";
            property int minWidth: 20
        }

        TableModelColumn {
            display: "Timestamp";
            property int minWidth: timestampMetrics.width + columnSpacing
        }

        TableModelColumn {
            display: "Timecode";
            decoration: "Timecode: Jump/Repeat";
            property int minWidth: timecodeMetrics.width + columnSpacing
        }

        TableModelColumn {
            display: "Recording Time"
            decoration: "Recording Time: Jump/Repeat";
            edit: "Recording Marks"
            property int minWidth: recordingTimeMetrics.width + columnSpacing
        }

        TableModelColumn {
            display: "Video/Audio";
            property int minWidth: 200 + columnSpacing + timecodeMetrics.height * 2
        }
    }
}
