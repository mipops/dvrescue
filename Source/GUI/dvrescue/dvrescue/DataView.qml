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
                column: dataModel.timecodeColumn

                JumpRepeatTextDelegate {
                    height: tableView.delegateHeight
                    implicitHeight: tableView.delegateHeight
                    property color evenColor: '#e3e3e3'
                    property color oddColor: '#f3f3f3'
                    property color redColor: 'red'
                    textFont.pixelSize: 13
                    text: display
                    hasJump: decoration.x
                    hasRepeat: decoration.y

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

            DelegateChoice {
                column: dataModel.recordingTimeColumn

                JumpRepeatTextDelegate {
                    height: tableView.delegateHeight
                    implicitHeight: tableView.delegateHeight
                    property color evenColor: '#e3e3e3'
                    property color oddColor: '#f3f3f3'
                    property color redColor: 'red'
                    textFont.pixelSize: 13
                    text: display
                    hasJump: decoration.x
                    hasRepeat: decoration.y
                    property var editRole: edit

                    color: (row % 2) == 0 ? evenColor : oddColor
                    overlayVisible: {
                        var sourceRow = sortFilterTableModel.toSourceRowIndex(row);
                        var frameNumber = cppDataModel.frameByIndex(sourceRow);
                        // var frameNumber = dataModel.getRow(sourceRow)[0]; // slow approach
                        return frameNumber === framePos
                    }
                    overlayColor: 'red'

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
                        onClicked: {
                            var sourceRow = sortFilterTableModel.toSourceRowIndex(row);
                            var frameNumber = cppDataModel.frameByIndex(sourceRow);
                            dataView.tapped(frameNumber);
                        }
                    }
                }
            }

            DelegateChoice {
                column: dataModel.arbitraryBitsColumn

                JumpRepeatTextDelegate {
                    height: tableView.delegateHeight
                    implicitHeight: tableView.delegateHeight
                    property color evenColor: '#e3e3e3'
                    property color oddColor: '#f3f3f3'
                    property color redColor: 'red'
                    textFont.pixelSize: 13
                    text: display
                    hasJump: decoration.x
                    hasRepeat: decoration.y

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

            DelegateChoice  {
                column: dataModel.videoAudioColumn

                SubstantialTextDelegate {
                    height: tableView.delegateHeight
                    implicitHeight: tableView.delegateHeight
                    property color evenColor: '#e3e3e3'
                    property color oddColor: '#f3f3f3'
                    property color redColor: 'red'
                    textFont.pixelSize: 13
                    text: display

                    property int sourceRow: sortFilterTableModel.toSourceRowIndex(row)
                    property int frameNumber: cppDataModel.frameByIndex(sourceRow)
                    property bool isSubstantialFrame: cppDataModel.isSubstantialFrame(sourceRow)
                    imageVisible: isSubstantialFrame

                    color: (row % 2) == 0 ? evenColor : oddColor
                    overlayVisible: {
                        // var frameNumber = dataModel.getRow(sourceRow)[0]; // slow approach
                        return frameNumber === framePos
                    }
                    overlayColor: 'red'

                    MouseArea {
                        id: videoAudioMouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked: {
                            dataView.tapped(frameNumber);
                        }
                    }

                    ToolTip {
                        visible: videoAudioMouseArea.containsMouse && isSubstantialFrame
                        text: isSubstantialFrame ? (cppDataModel.getLastSubstantialFrameTransition(sourceRow)) : ""
                        anchors.centerIn: parent
                    }
                }
            }

            DelegateChoice {
                column: dataModel.videoErrorColumn

                OddEvenTextDelegate {
                    height: tableView.delegateHeight
                    implicitHeight: tableView.delegateHeight
                    property color evenColor: '#e3e3e3'
                    property color oddColor: '#f3f3f3'
                    property color redColor: 'red'
                    textFont.pixelSize: 13
                    text: edit ? "100%" : display

                    color: (row % 2) == 0 ? evenColor : oddColor
                    overlayVisible: {
                        var sourceRow = sortFilterTableModel.toSourceRowIndex(row);
                        var frameNumber = cppDataModel.frameByIndex(sourceRow);
                        // var frameNumber = dataModel.getRow(sourceRow)[0]; // slow approach
                        return frameNumber === framePos
                    }
                    overlayColor: 'red'

                    evenProgressColor: edit ? 'yellow' : 'darkgreen'
                    oddProgressColor: edit ? 'yellow' : 'green'
                    evenProgress.value: edit ? 1 : decoration.x
                    oddProgress.value: edit ? 1 : decoration.y

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
                column: dataModel.audioErrorColumn

                OddEvenTextDelegate {
                    height: tableView.delegateHeight
                    implicitHeight: tableView.delegateHeight
                    property color evenColor: '#e3e3e3'
                    property color oddColor: '#f3f3f3'
                    property color redColor: 'red'
                    textFont.pixelSize: 13
                    text: edit ? "100%" : display

                    color: (row % 2) == 0 ? evenColor : oddColor
                    overlayVisible: {
                        var sourceRow = sortFilterTableModel.toSourceRowIndex(row);
                        var frameNumber = cppDataModel.frameByIndex(sourceRow);
                        // var frameNumber = dataModel.getRow(sourceRow)[0]; // slow approach
                        return frameNumber === framePos
                    }
                    overlayColor: 'red'

                    evenProgressColor: edit ? 'yellow' : 'darkblue'
                    oddProgressColor: edit ? 'yellow' : 'blue'
                    evenProgress.value: edit ? 1 : decoration.x
                    oddProgress.value: edit ? 1 : decoration.y

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
                column: dataModel.captionsColumn

                IconDelegate {
                    height: tableView.delegateHeight
                    implicitHeight: tableView.delegateHeight
                    property color evenColor: '#e3e3e3'
                    property color oddColor: '#f3f3f3'
                    property color redColor: 'red'

                    property string imageUrl: {
                        if(display === '┬') {
                            return decoration ? "icons/closed-caption-start-error.svg" : "icons/closed-caption-start.svg"
                        } else if(display === '┴') {
                            return decoration ? "icons/closed-caption-end-error.svg" : "icons/closed-caption-end.svg"
                        } else if(display === '│' || display === 'y') {
                            return decoration ? "icons/closed-caption-middle-error.svg" : "icons/closed-caption-middle.svg"
                        }

                        return null;
                    }
                    imageVisible: imageUrl !== null
                    Binding on imageSource {
                        when: imageUrl != null
                        value: imageUrl
                    }

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

            DelegateChoice {
                column: dataModel.absoluteTrackNumberColumn

                JumpRepeatTextDelegate {
                    height: tableView.delegateHeight
                    implicitHeight: tableView.delegateHeight
                    property color evenColor: '#e3e3e3'
                    property color oddColor: '#f3f3f3'
                    property color redColor: 'red'
                    textFont.pixelSize: 13
                    text: display
                    hasJump: decoration.x
                    hasRepeat: decoration.y

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

        property int timecodeColumn: columnsNames.indexOf("Timecode");
        property int recordingTimeColumn: columnsNames.indexOf("Recording Time");
        property int arbitraryBitsColumn: columnsNames.indexOf("Arbitrary Bits");
        property int captionsColumn: columnsNames.indexOf("CC");

        property int videoAudioColumn: columnsNames.indexOf("Video/Audio");

        property int videoErrorColumn: columnsNames.indexOf("Video Error %");
        property int audioErrorColumn: columnsNames.indexOf("Audio Error %");
        property int absoluteTrackNumberColumn: columnsNames.indexOf("Absolute Track Number")

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
            property int minWidth: timecodeMetrics.width + columnSpacing + timecodeMetrics.height * 2.5
        }

        TableModelColumn {
            display: "Recording Time"
            decoration: "Recording Time: Jump/Repeat";
            edit: "Recording Marks"
            property int minWidth: recordingTimeMetrics.width + columnSpacing + timecodeMetrics.height * 2.5
        }

        TableModelColumn {
            display: "Video Error %";
            decoration: "Video Error";
            edit: "Video Error/Full Concealment";
            property int minWidth: errorConcealmentMetrics.width
        }

        TableModelColumn {
            display: "Audio Error %";
            decoration: "Audio Error";
            edit: "Audio Error/Full Concealment";
            property int minWidth: errorConcealmentMetrics.width
        }

        TableModelColumn {
            display: "Full Concealment";
            property int minWidth: 20
        }

        TableModelColumn {
            display: "Missing Packs";
            property int minWidth: missingPacksMetrics.width + columnSpacing
        }

        TableModelColumn {
            display: "Arbitrary Bits";
            decoration: "Arbitrary Bits: Jump/Repeat"
            property int minWidth: 20 + columnSpacing + timecodeMetrics.height * 2
        }

        TableModelColumn {
            display: "CC";
            decoration: "CC/Mismatch"
            property int minWidth: 40
        }

        TableModelColumn {
            display: "Byte Offset";
            property int minWidth: 20
        }

        TableModelColumn {
            display: "Video/Audio";
            property int minWidth: 200 + columnSpacing + timecodeMetrics.height * 2
        }

        TableModelColumn {
            display: "Absolute Track Number";
            decoration: "Absolute Track Number: Jump/Repeat"
            property int minWidth: 20 + columnSpacing + timecodeMetrics.height * 2
        }
    }
}
