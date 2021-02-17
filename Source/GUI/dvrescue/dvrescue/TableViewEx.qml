import QtQuick 2.0
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtQuick 2.12

Item {

    property int delegateHeight: 25
    property alias view: tableView
    property alias model: tableView.model
    property alias delegate: tableView.delegate
    property alias headerDelegate: header.delegate
    function getHeaderItem(index) {
        return header.itemAt(index);
    }

    property alias columnWidthProvider: tableView.columnWidthProvider

    function bringToView(index) {        
        console.debug('bringingToView: ', index);

        var expectedContentY = (index) * (delegateHeight + tableView.rowSpacing) - tableView.topMargin
        var maxContentY = model.rowCount === 0 ? 0 :
                                                 (model.rowCount * (delegateHeight + tableView.rowSpacing) - tableView.rowSpacing - tableView.height - tableView.topMargin)

        if(tableView.contentHeight < tableView.height)
            return;

        if(maxContentY >= 0) {
            console.debug('adjusting contentY...');
            tableView.contentY = Math.min(expectedContentY, maxContentY)
            console.debug('adjusting contentY... done: ', tableView.contentY);
        }
    }

    clip: true

    function forceLayout() {
        console.debug('forceLayout')
        tableView.forceLayout();
    }

    TableView {
        id: tableView
        columnSpacing: 1
        rowSpacing: 1
        clip: true
        flickableDirection: Flickable.VerticalFlick
        topMargin: columnsHeader.implicitHeight

        onWidthChanged: {
            console.debug('tableView.width: ', width)
        }

        Row {
            id: columnsHeader
            y: tableView.contentY
            spacing: tableView.columnSpacing
            x: 0
            z: 2

            onWidthChanged: {
                console.debug('rowLayout.width: ', width)
            }

            Repeater {
                id: header
                model: tableView.columns > 0 ? tableView.columns : 1
            }
        }

        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right

        columnWidthProvider: function(column) {
            return tableView.model ? (tableView.width) / tableView.model.columnCount : 0;
        }

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
    }
}
