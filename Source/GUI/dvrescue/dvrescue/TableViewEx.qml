import QtQuick 2.0
import QtQuick.Controls 2.12
import QtQuick 2.12
import Qt.labs.qmlmodels 1.0

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

        var expectedContentY = (index) * (delegateHeight + tableView.rowSpacing)
        var maxContentY = model.rowCount === 0 ? 0 :
                                                 (model.rowCount * (delegateHeight + tableView.rowSpacing) - tableView.rowSpacing - tableView.height)

        if(tableView.contentHeight < tableView.height)
            return;

        if(maxContentY >= 0) {
            console.debug('adjusting contentY...');
            tableView.contentY = Math.min(expectedContentY, maxContentY)
            console.debug('adjusting contentY... done!');
        }
    }

    clip: true

    function forceLayout() {
        tableView.forceLayout();
    }

    Row {
        id: columnsHeader
        // y: tableView.contentY
        spacing: 1

        Repeater {
            id: header
            model: tableView.columns > 0 ? tableView.columns : 1
        }
    }

    TableView {
        id: tableView
        columnSpacing: 1
        rowSpacing: 1
        clip: true
        flickableDirection: Flickable.VerticalFlick
        // topMargin: columnsHeader.implicitHeight

        anchors.top: columnsHeader.bottom
        anchors.bottom: parent.bottom;
        anchors.left: parent.left
        anchors.right: parent.right

        columnWidthProvider: function(column) {
            return tableView.model ? (tableView.width) / tableView.model.columnCount : 0;
        }

        ScrollBar.vertical: ScrollBar {
            id: vscroll
        }

        ScrollIndicator.horizontal: ScrollIndicator { }
        ScrollIndicator.vertical: ScrollIndicator { }
    }
}
