import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.12
import Qt.labs.settings 1.0
import Launcher 0.1
import FileUtils 1.0

FileDialog {
    id: specifyPathDialog
    selectExisting: false
    property var callback;

    onAccepted: {
        var fileUrl = specifyPathDialog.fileUrl;
        console.debug('selected file: ', fileUrl);
        if(callback)
            callback(fileUrl);
    }
}
