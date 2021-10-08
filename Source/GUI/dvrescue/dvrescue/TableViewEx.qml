import QtQuick 2.0
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtQuick 2.12

Item {

    property int delegateHeight: 25
    property alias view: tableView
    property alias model: tableView.model
    property var dataModel: null
    property alias delegate: tableView.delegate
    property alias headerDelegate: header.delegate
    function getHeaderItem(index) {
        return header.itemAt(index);
    }

    property alias columnWidthProvider: tableView.columnWidthProvider

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

    function getMaxDesiredHeight() {
        var value = 0
        for(var i = 0; i < model.columnCount; ++i)
        {
            var headerItem = getHeaderItem(i)
            if(headerItem === null) {
                continue;
            }

            value = Math.max(value, headerItem.desiredHeight)
        }
        return value;
    }

    function getTotalDesiredWidth() {
        var value = 0
        for(var i = 0; i < model.columnCount; ++i)
        {
            var headerItem = getHeaderItem(i)
            if(headerItem === null) {
                continue;
            }

            var minWidth = dataModel.columns[i].minWidth
            value += Math.max(headerItem.desiredWidth, minWidth)
        }

        return value;
    }

    function getColumnWidth(column) {
        var headerItem = getHeaderItem(column);
        if(headerItem === null)
            return 0;

        var minWidth = dataModel.columns[column].minWidth
        var desiredWidth = Math.max(headerItem.desiredWidth, minWidth)

        var relativeWidth = totalDesiredWidth !== 0 ? (desiredWidth / totalDesiredWidth) : 0
        var allowedWidth = relativeWidth * width

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
        for(var i = 0; i < model.columnCount; ++i)
        {
            newColumnWidths[i] = getColumnWidth(i)
            totalWidth += newColumnWidths[i]
        }

        totalWidth += (model.columnCount - 1) * view.columnSpacing
        columnWidths = newColumnWidths

        view.contentWidth = totalWidth
    }

    property var columnWidths: ({})
    onWidthChanged: {
        adjustColumnWidths()
    }

    columnWidthProvider: function(column) {
        return columnWidths[column];
    }

    Component.onCompleted: {
        Qt.callLater(() => {
                         initialized = true;
                         adjustColumnWidths();
                     })
    }

    clip: true

    function forceLayout() {
        tableView.forceLayout();
    }

    TableView {
        id: tableView
        columnSpacing: 1
        rowSpacing: 1
        clip: true
        flickableDirection: Flickable.VerticalFlick
        topMargin: columnsHeader.implicitHeight

        Row {
            id: columnsHeader
            y: tableView.contentY
            spacing: tableView.columnSpacing
            x: 0
            z: 2

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
