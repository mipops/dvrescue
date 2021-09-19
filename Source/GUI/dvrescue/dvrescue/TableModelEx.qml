import QtQuick 2.0
import TableModel 1.0

TableModel {
    property var columnsNames: {
        var names = [];
        for(var i = 0; i < columns.length; ++i) {
            names.push(columns[i].display)
        }
        return names;
    }
}
