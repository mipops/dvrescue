import QtQuick.Dialogs as Dialogs

Dialogs.FileDialog {
    property bool selectExisting: true;
    property bool selectMultiple: false;
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
