import QtQuick.Dialogs as Dialogs

Dialogs.FileDialog {
    property bool selectExisting: true;
    property bool selectMultiple: false;
    property url fileUrl;
    property var fileUrls;
    onSelectedFileChanged: {
        fileUrl = selectedFile
    }
    onSelectedFilesChanged: {
        fileUrls = selectedFiles
    }

    fileMode: {
        var mode;
        if(selectExisting || selectMultiple) {
            if(selectExisting)
                mode = FileDialog.OpenFile;
            if(selectMultiple)
                mode = FileDialog.OpenFiles;
        } else {
            mode = FileDialog.SaveFile;
        }
        return mode;
    }
}
