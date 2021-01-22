import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.12
import Qt.labs.settings 1.0
import Launcher 0.1
import FileUtils 1.0

FileDialog {
    id: selectPathDialog
    selectExisting: true
    property var callback;

    onAccepted: {
        var fileUrl = selectPathDialog.fileUrl;
        console.debug('selected file: ', fileUrl);
        if(callback)
            callback(fileUrl);
    }
}
