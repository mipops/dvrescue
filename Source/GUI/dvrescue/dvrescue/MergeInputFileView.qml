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

    signal deleteClicked(var row);

    property alias dataModel: dataModel
    property alias tableView: tableView

    readonly property string filePathColumn: "Input File Path"
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

    function newRow(path) {
        for(var i = 0; i < dataModel.rowCount; ++i) {
            var row = dataModel.getRow(i)
            if(row[filePathColumn] === path)
                return;
        }

        var rowEntry = {}
        rowEntry[filePathColumn] = path

        dataModel.appendRow(rowEntry)
    }

    function scrollToTop() {
        tableView.view.contentY = -16
    }

    Text {
        id: label
        text: 'FILES TO MERGE'
        color: 'white'
        font.pixelSize: 20
    }

    Button {
        anchors.left: label.right
        anchors.leftMargin: 20

        anchors.top: label.top
        anchors.topMargin: 2
        anchors.bottom: label.bottom
        anchors.bottomMargin: 2

        text: "Clear"
        onClicked: {
            dataModel.clear();
        }
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

            Rectangle {
                id: dropAreaHighlight
                visible: false
                anchors.fill: parent
                color: 'transparent'
                border.color: 'green'
                border.width: 2
            }

            DropArea {
                id: dropArea;
                anchors.fill: parent
                enabled: true

                onEntered: {
                    dropAreaHighlight.visible = true;
                    drag.accept (Qt.LinkAction);
                }
                onDropped: {
                    dropAreaHighlight.visible = false;
                    if(drop.urls.length !== 0)
                    {
                        drop.urls.forEach((url) => {
                                              var filePath = FileUtils.getFilePath(url);
                                              root.newRow(filePath);
                                          })
                    }

                    console.log(drop.urls)
                }
                onExited: {
                    dropAreaHighlight.visible = false;
                }
            }

            Menu {
                id: contextMenu
                property int currentRow: -1

                MenuItem {
                    text: "Show location"
                    onClicked: {
                        var filePath = dataModel.getRow(contextMenu.currentRow)[filePathColumn];
                        Qt.openUrlExternally('file:///' + FileUtils.getFileDir(filePath))
                    }
                }

                function showBelowControl(control) {
                    var mapped = control.mapToItem(tableView, 0, 0);
                    contextMenu.x = mapped.x
                    contextMenu.y = mapped.y + control.height

                    contextMenu.open()
                }

                function show(pos, row) {
                    contextMenu.x = pos.x
                    contextMenu.y = pos.y
                    currentRow = row

                    contextMenu.open()
                }
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
                    column: 0

                    TextDelegate {
                        height: tableView.delegateHeight
                        implicitHeight: tableView.delegateHeight
                        property color evenColor: '#e3e3e3'
                        property color oddColor: '#f3f3f3'
                        property color redColor: 'red'
                        textFont.pixelSize: 13
                        text: display

                        anchors.left: parent.left
                        anchors.leftMargin: deleteButton.width

                        color: (row % 2) == 0 ? evenColor : oddColor
                        overlayVisible: true
                        overlayColor: row < colors.length ? colors[row] : 'transparent'

                        CustomButton {
                            id: deleteButton
                            anchors.left: parent.left
                            anchors.leftMargin: -deleteButton.width

                            icon.color: "black"
                            icon.source: "/icons/exit.svg"
                            implicitHeight: parent.height
                            implicitWidth: implicitHeight

                            onClicked: {
                                dataModel.removeRow(row, 1);
                            }
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

    property int columnSpacing: 10

    TableModelEx {
        id: dataModel

        TableModelColumn {
            display: "Input File Path"
            property int minWidth: 250
        }
    }
}
