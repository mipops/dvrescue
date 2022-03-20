import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import Qt.labs.qmlmodels 1.0

Dialog {
    id: root

    modal: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    parent: Overlay.overlay
    anchors.centerIn: parent
    width: 1350
    height: 800

    property bool canPrev: true
    property bool canNext: true

    signal prev();
    signal next();

    onClosed: {
        imageSource = null
    }

    property string imageSource
    property var data
    onDataChanged: {

        tableView.model.clear()
        var rows = data.rows
        rows.forEach((row) => {
                         var cell0 = row.cells[0]
                         var cell1 = row.cells[1]

                         var rowEntry = {}
                         rowEntry['value1'] = cell0.value
                         rowEntry['value2'] = cell1.value
                         rowEntry['value2Color'] = '#' + cell1.color

                         tableView.model.appendRow(rowEntry)
                     });
    }

    ColumnLayout {
        id: layout
        anchors.fill: parent

        RowLayout {

            Item {
                Layout.minimumHeight: 300
                Layout.minimumWidth: 300

                Image {
                    id: image
                    anchors.fill: parent
                    fillMode: Image.PreserveAspectFit
                    source: imageSource
                }

                Rectangle {
                    anchors.fill: parent

                    color: 'white'
                    visible: image.status !== Image.Ready

                    BusyIndicator {
                        anchors.centerIn: parent
                    }
                }
            }

            Label {
                text: 'frame metadata tbd'
                Layout.fillWidth: true
            }

            Layout.fillWidth: true
            Layout.maximumHeight: 300
        }

        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            Button {
                text: "<"
                enabled: canPrev
                onClicked: prev()
            }
            Button {
                text: ">"
                enabled: canNext
                onClicked: next()
            }
            Button {
                text: "Deselect all"
            }
        }

        TableView {
            id: tableView
            Layout.fillHeight: true
            Layout.fillWidth: true

            onWidthChanged: {
                forceLayout();
            }

            columnWidthProvider: function(column) {
                if(column === 0)
                    return 50
                return width - 50
            }

            property int delegateHeight: 25

            ScrollBar.vertical: ScrollBar {
                id: vscroll
                policy: ScrollBar.AsNeeded
            }

            ScrollBar.horizontal: ScrollBar {
                id: hscroll
                policy: ScrollBar.AsNeeded
            }

            ScrollIndicator.horizontal: ScrollIndicator { }
            ScrollIndicator.vertical: ScrollIndicator { }

            model: TableModelEx {
                id: dataModel

                TableModelColumn {
                    display: "value1"
                }

                TableModelColumn {
                    display: "value2"
                    decoration: "value2Color"
                }
            }

            delegate: DelegateChooser {
                DelegateChoice  {
                    Rectangle {
                        id: textDelegate
                        implicitWidth: 100
                        height: tableView.delegateHeight
                        implicitHeight: tableView.delegateHeight

                        color: (row % 2) == 0 ? 'white' : '#fefefe'
                        property real overlayColorOpacity: 0.5
                        property alias overlayColor: overlay.color
                        property alias overlayVisible: overlay.visible
                        property alias text: textLabel.text
                        property alias textFont: textLabel.font

                        TextInput {
                            id: textLabel
                            text: display
                            anchors.verticalCenter: parent.verticalCenter
                            readOnly: true
                            font.bold: column === 0
                            font.pixelSize: 13
                            font.family: "Courier New"
                            color: column === 0 ? 'black' : decoration
                        }

                        Rectangle {
                            id: overlay
                            anchors.fill: parent
                            opacity: overlayColorOpacity
                            visible: false
                        }
                    }
                }
            }
        }

    }
}


