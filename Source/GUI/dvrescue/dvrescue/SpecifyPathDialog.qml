import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12
import Qt.labs.settings 1.0
import Qt.labs.platform 1.0 as Platform
import Dialogs 1.0
import Launcher 0.1
import FileUtils 1.0

Dialog {
    id: specifyPathDialog
    property var callback;
    property var filePath;
    property var fileName;
    property var fileUrl;
    property string extension: '.dv';

    standardButtons: Dialog.Ok | Dialog.Cancel

    Dialog {
        id: overwriteFileDialog

        standardButtons: Dialog.Yes | Dialog.No

        Label {
            text: qsTr("<b>File already exists, overwrite?</b>")
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
        }

        onAccepted: {
            if (specifyPathDialog.callback)
                specifyPathDialog.callback(specifyPathDialog.fileUrl)
        }

        onRejected: {
            specifyPathDialog.open();
        }
    }

    Platform.FolderDialog {
        id: selectFolderDialog

        onAccepted: {
            specifyPathDialog.filePath = folder;
            specifyPathDialog.visible = true;
        }
        onRejected: {
            specifyPathDialog.visible = true;
        }
    }


    GridLayout {
        columns: 2
        Label {
            text: qsTr("<b>Output directory: </b>")
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
        }
        Button {
            id: folderNameButton
            Layout.fillWidth: true
            text: FileUtils.getFileName(filePath)
            onClicked: {
                specifyPathDialog.visible = false;
                console.debug("open folder dialog");
                selectFolderDialog.folder = filePath;
                selectFolderDialog.open();
            }

        }
        Label {
            text: qsTr("<b>Output filename: </b>")
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
        }
        TextField {
            id: fileNameField
            Layout.fillWidth: true
            placeholderText: fileName
        }
    }

    onReset: {
        fileNameField.text = '';
    }

    onAccepted: {
        fileUrl = filePath + '/' + (fileNameField.text ? fileNameField.text : fileName);
        if (!fileUrl.endsWith(extension))
            fileUrl += extension;

        console.debug('selected file: ', fileUrl);
        if (FileUtils.exists(FileUtils.getFilePath(fileUrl, true))) {
            overwriteFileDialog.open();
        } else {
            if(callback)
                callback(fileUrl);
        }
    }
}
