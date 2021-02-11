import QtQuick 2.0
import QtQuick 2.12
import TableModel 1.0
import TableModelColumn 1.0
import SortFilterTableModel 1.0
import QtQuick.Controls 2.12
import Qt.labs.qmlmodels 1.0

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
        delegateHeight: 25

        headerDelegate: SortableFiltrableColumnHeading {
            id: header
            width: tableView.columnWidths[modelData]
            text: dataModel.columns[modelData].display
            canFilter: true
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
            DelegateChoice {
                column: 22

                OddEvenTextDelegate {
                    height: tableView.delegateHeight
                    implicitHeight: tableView.delegateHeight
                    property color evenColor: '#e3e3e3'
                    property color oddColor: '#f3f3f3'
                    property color redColor: 'red'
                    textFont.pixelSize: 13
                    text: display

                    color: (row % 2) == 0 ? evenColor : oddColor
                    overlayVisible: {
                        var sourceRow = sortFilterTableModel.toSourceRowIndex(row);
                        var frameNumber = cppDataModel.frameByIndex(sourceRow);
                        // var frameNumber = dataModel.getRow(sourceRow)[0]; // slow approach
                        return frameNumber === framePos
                    }
                    overlayColor: 'red'

                    evenProgressColor: 'darkgreen'
                    oddProgressColor: 'green'
                    evenProgress.value: decoration.x
                    oddProgress.value: decoration.y

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            var sourceRow = sortFilterTableModel.toSourceRowIndex(row);
                            var frameNumber = cppDataModel.frameByIndex(sourceRow);
                            dataView.tapped(frameNumber);
                        }
                    }
                }
            }

            DelegateChoice {
                column: 23

                OddEvenTextDelegate {
                    height: tableView.delegateHeight
                    implicitHeight: tableView.delegateHeight
                    property color evenColor: '#e3e3e3'
                    property color oddColor: '#f3f3f3'
                    property color redColor: 'red'
                    textFont.pixelSize: 13
                    text: display

                    color: (row % 2) == 0 ? evenColor : oddColor
                    overlayVisible: {
                        var sourceRow = sortFilterTableModel.toSourceRowIndex(row);
                        var frameNumber = cppDataModel.frameByIndex(sourceRow);
                        // var frameNumber = dataModel.getRow(sourceRow)[0]; // slow approach
                        return frameNumber === framePos
                    }
                    overlayColor: 'red'

                    evenProgressColor: 'darkblue'
                    oddProgressColor: 'blue'
                    evenProgress.value: decoration.x
                    oddProgress.value: decoration.y

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            var sourceRow = sortFilterTableModel.toSourceRowIndex(row);
                            var frameNumber = cppDataModel.frameByIndex(sourceRow);
                            dataView.tapped(frameNumber);
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

                    color: (row % 2) == 0 ? evenColor : oddColor
                    overlayVisible: {
                        var sourceRow = sortFilterTableModel.toSourceRowIndex(row);
                        var frameNumber = cppDataModel.frameByIndex(sourceRow);
                        // var frameNumber = dataModel.getRow(sourceRow)[0]; // slow approach
                        return frameNumber === framePos
                    }
                    overlayColor: 'red'

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            var sourceRow = sortFilterTableModel.toSourceRowIndex(row);
                            var frameNumber = cppDataModel.frameByIndex(sourceRow);
                            dataView.tapped(frameNumber);
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
            display: "Recording Marks";
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
            display: "Missing Packs";
            property int minWidth: missingPacksMetrics.width + columnSpacing
        }

        TableModelColumn {
            display: "Full Concealment";
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
            display: "Audio";
            property int minWidth: 60
        }

        TableModelColumn {
            display: "Video Error Concealment %";
            decoration: "Video Error Concealment";
            property int minWidth: errorConcealmentMetrics.width
        }

        TableModelColumn {
            display: "Audio Error %";
            decoration: "Audio Error";
            property int minWidth: errorConcealmentMetrics.width
        }
    }
}
