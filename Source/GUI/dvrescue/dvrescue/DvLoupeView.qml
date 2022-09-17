import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import TableModel 1.0
import TableModelColumn 1.0
import Qt.labs.qmlmodels 1.0

Dialog {
    id: root

    signal selectionChanged();
    property alias dataModel: dataModel
    modal: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    parent: Overlay.overlay
    width: 1340
    height: 760
    clip: true
    padding: 5
    topPadding: 5
    spacing: 0

    property bool canPrev: true
    property bool canNext: true

    property alias headerCheckboxChecked: headerCheckbox.checked
    property alias subcodeCheckboxChecked: subcodeCheckbox.checked
    property alias vauxCheckboxChecked: vauxCheckbox.checked
    property alias audioCheckboxChecked: audioCheckbox.checked
    property alias videoCheckboxChecked: videoCheckbox.checked
    property alias allCheckboxChecked: allCheckbox.checked
    property alias errorOnlyCheckboxChecked: errorOnlyCheckbox.checked

    signal prev();
    signal next();
    signal refresh();

    onClosed: {
        imageSource = null
    }

    Shortcut {
        enabled: root.visible
        sequence: StandardKey.MoveToPreviousLine
        onActivated: {
            vscroll.decrease();
        }
    }
    Shortcut {
        enabled: root.visible
        sequence: StandardKey.MoveToNextLine
        onActivated: {
            vscroll.increase();
        }
    }
    Shortcut {
        enabled: root.visible
        sequence: StandardKey.MoveToPreviousPage
        onActivated: {
            vscroll.position -= tableView.height / tableView.contentHeight;
            if(vscroll.position < 0)
                vscroll.position = 0;
        }
    }
    Shortcut {
        enabled: root.visible
        sequence: StandardKey.MoveToNextPage
        onActivated: {
            vscroll.position += tableView.height / tableView.contentHeight;
            if(vscroll.position > 1)
                vscroll.position = 1;
        }
    }

    property string imageSource
    property var data
    property var indexByVbl: ({})
    onDataChanged: {

        indexByVbl = {};
        tableView.model.clear()
        var rows = data.rows
        rows.forEach((row) => {
                         var cell0 = row.cells[0]
                         var cell1 = row.cells[1]

                         var rowEntry = {}
                         rowEntry['value1'] = cell0.value
                         rowEntry['value2'] = cell1.value
                         rowEntry['value2Color'] = '#' + cell1.color
                         rowEntry['selected'] = false

                         if(cell1.hasOwnProperty('loc'))
                         {
                             var loc = cell1['loc']
                             var splitted = loc.split(':');
                             var w = splitted[0]
                             var h = splitted[1]
                             var x = splitted[2]
                             var y = splitted[3]

                             rowEntry['x'] = x;
                             rowEntry['y'] = y;
                             rowEntry['xw'] = x + w;
                             rowEntry['yh'] = y + h;
                         }

                         if(cell1.hasOwnProperty('vbl'))
                         {
                             var vbl = cell1['vbl']
                             var nextIndex = tableView.model.rowCount

                             rowEntry['vbl'] = vbl
                             indexByVbl[vbl] = nextIndex
                         }

                         rowEntry['blockNumber'] = tableView.model.rowCount
                         tableView.model.appendRow(rowEntry)
                     });
    }

    ButtonGroup {
        buttons: [allCheckbox, errorOnlyCheckbox]
    }

    ColumnLayout {
        id: layout
        anchors.fill: parent
        spacing: 0

        RowLayout {
            id: imageLayout

            Item {
                Layout.minimumWidth: 720
                Layout.minimumHeight: 480

                Image {
                    id: image
                    anchors.fill: parent
                    fillMode: Image.PreserveAspectFit
                    source: imageSource
                }

                MouseArea {
                    x: image.width / 2 - image.paintedWidth / 2
                    y: image.height / 2 - image.paintedHeight / 2
                    width: image.paintedWidth
                    height: image.paintedHeight

                    onPressed: { pos = Qt.point(mouse.x, mouse.y) }
                    onPositionChanged: {
                        var diff = Qt.point(mouse.x - pos.x, mouse.y - pos.y)
                        root.x += diff.x
                        root.y += diff.y
                    }
                    property point pos

                    onClicked: {
                        var cx = mouse.x
                        var cy = mouse.y

                        console.debug('cx: ', cx, 'cy:', cy)
                        for(var i = 0; i < tableView.model.rowCount; ++i) {
                            var rowData = dataModel.getRow(i);
                            if(!rowData.x)
                                continue;

                            var x = rowData.x
                            var xw = rowData.xw
                            var y = rowData.y
                            var yh = rowData.yh

                            if(x <= cx && cx < xw && y <= cy && cy < yh) {
                                var rowIndex = i
                                console.debug('rowIndex: ', rowIndex);

                                if(rowData.hasOwnProperty('vbl')) {

                                    rowIndex = indexByVbl[rowData.vbl]
                                    rowData = dataModel.getRow(rowIndex)

                                    rowData.selected = !rowData.selected;
                                    dataModel.setRow(rowIndex, rowData)
                                    root.selectionChanged()
                                    tableView.bringToView(rowIndex)

                                    break;
                                }
                            }
                        }
                    }
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
                text: ''
                Layout.fillWidth: true
            }

            Layout.fillWidth: true
            Layout.maximumHeight: 480
        }

        Item {
            Layout.fillWidth: true
            height: buttonsLayout.height

            RowLayout {
                id: buttonsLayout
                anchors.right: parent.right
                Layout.alignment: Qt.AlignRight

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
                    onClicked: {

                        var hasSelectionChanges = false;
                        for(var i = 0; i < dataModel.rowCount; ++i) {
                            var rowData = dataModel.getRow(i);
                            if(rowData.selected) {
                                hasSelectionChanges = true;
                                rowData.selected = false;
                                dataModel.setRow(i, rowData)
                            }
                        }

                        if(hasSelectionChanges)
                            selectionChanged();
                    }
                }
            }

            RowLayout {
                anchors.left: parent.left

                Label {
                    text: "Show DV DIF block types: "
                }

                Button {
                    id: headerCheckbox
                    checkable: true
                    text: "Header"
                    onCheckedChanged: {
                        refresh();
                    }
                }

                Button {
                    id: subcodeCheckbox
                    checkable: true
                    text: "Subcode"
                    onCheckedChanged: {
                        refresh();
                    }
                }

                Button {
                    id: vauxCheckbox
                    checkable: true
                    text: "Vaux"
                    onCheckedChanged: {
                        refresh();
                    }
                }

                Button {
                    id: audioCheckbox
                    checkable: true
                    text: "Audio"
                    onCheckedChanged: {
                        refresh();
                    }
                }

                Button {
                    id: videoCheckbox
                    checkable: true
                    text: "Video"
                    onCheckedChanged: {
                        refresh();
                    }
                }

                Label {
                    text: "Show Video DIF blocks: "
                }

                RadioButton {
                    id: allCheckbox
                    text: "All"
                    checked: true
                    onCheckedChanged: {
                        refresh();
                    }
                }

                RadioButton {
                    id: errorOnlyCheckbox
                    text: "Errors Only"
                    onCheckedChanged: {
                        refresh();
                    }
                }
            }
        }


        TableView {
            id: tableView
            Layout.fillHeight: true
            Layout.fillWidth: true
            clip: true

            function bringToView(index) {
                console.debug('bringingToView: ', index, 'topMargin: ', tableView.topMargin);

                if(index === 0) {
                    tableView.contentY = -tableView.topMargin
                    return
                }

                var expectedContentY = (index) * (delegateHeight + tableView.rowSpacing) - tableView.topMargin
                var maxContentY = model.rowCount === 0 ? 0 :
                                                         (model.rowCount * (delegateHeight + tableView.rowSpacing) - tableView.rowSpacing - tableView.height - tableView.topMargin)

                if(tableView.contentHeight < tableView.height)
                    return;

                if(maxContentY >= 0) {
                    // console.debug('adjusting contentY...');
                    tableView.contentY = Math.min(expectedContentY, maxContentY)
                    // console.debug('adjusting contentY... done: ', tableView.contentY);
                }
            }

            onWidthChanged: {
                forceLayout();
            }

            columnWidthProvider: function(column) {
                if(column === 0)
                    return 50
                if(column === 1)
                    return 50
                return width - 100
            }

            property int delegateHeight: 25

            ScrollBar.vertical: ScrollBar {
                id: vscroll
                policy: ScrollBar.AlwaysOn
                stepSize: tableView.delegateHeight / tableView.contentHeight
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
                    display: "blockNumber"
                    edit: "selected"
                }

                TableModelColumn {
                    display: "value1"
                    edit: "selected";
                }

                TableModelColumn {
                    display: "value2"
                    decoration: "value2Color"
                    edit: "selected";
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
                        property alias overlayVisible: overlay.visible
                        property alias text: textLabel.text
                        property alias textFont: textLabel.font

                        TextInput {
                            id: textLabel
                            text: display
                            anchors.verticalCenter: parent.verticalCenter
                            readOnly: true
                            font.bold: column === 0 || column === 1
                            font.pixelSize: 13
                            font.family: "Courier New"
                            color: (column === 0 || column === 1) ? 'black' : decoration
                        }

                        Rectangle {
                            id: overlay
                            anchors.fill: parent
                            opacity: overlayColorOpacity
                            color: 'purple'
                            visible: edit
                        }

                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                var rowData = dataModel.getRow(row);
                                console.debug('rowData: ', JSON.stringify(rowData, 0, 4))
                                if(rowData.hasOwnProperty('vbl')) {
                                    rowData.selected = !rowData.selected;
                                    dataModel.setRow(row, rowData)

                                    selectionChanged();
                                }
                            }
                        }
                    }
                }
            }
        }

    }

    ResizeRectangle {
        target: root
        anchors.fill: root.contentItem
        anchors.margins: -root.padding
    }
}


