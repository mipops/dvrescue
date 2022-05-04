import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Layouts 1.12
import Qt.labs.settings 1.0
import Dialogs 1.0
import Launcher 0.1
import FileUtils 1.0

FileDialog {
    id: specifyPathDialog
    // selectExisting: false
    property var callback;

    onAccepted: {
        var fileUrl = specifyPathDialog.fileUrl;
        console.debug('selected file: ', fileUrl);
        if(callback)
            callback(fileUrl);
    }
}
