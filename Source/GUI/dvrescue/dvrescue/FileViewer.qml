import QtQuick 2.12
import QtQuick.Layouts 1.11
import QtQuick.Controls 2.12
import FileUtils 1.0

Rectangle {
    id: fileViewer
    height: 100
    color: "#ffffff"

    property alias recentsPopup: recentsPopup
    property alias fileView: fileView
    property alias selectedPath: fileView.selectedPath
    property alias files: fileView.files
    property alias updated: fileView.updated

    SelectPathDialog {
        id: selectPath
        selectMultiple: true
        nameFilters: [
            "Report files (*.dvrescue.xml)",
            "Video files (*.mov *.mkv *.avi *.dv *.mxf)"
        ]
    }

    RecentsPopup {
        id: recentsPopup
    }

    Column {
        id: buttons
        anchors.right: parent.right
        anchors.top: parent.top

        Button {
            text: qsTr("Add files")
            onClicked: {
                selectPath.callback = (urls) => {
                    urls.forEach((url) => {
                                     fileView.add(FileUtils.getFilePath(url));
                                 });
                }

                selectPath.open();
            }
        }

        Button {
            text: qsTr("Recent")

            onClicked: {
                var mapped = mapToItem(fileViewer, 0, 0);
                recentsPopup.x = mapped.x - recentsPopup.width + width
                recentsPopup.y = mapped.y + height

                recentsPopup.open();
            }
        }
    }

    FileView {
        id: fileView
        anchors.left: parent.left
        anchors.right: buttons.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
    }
}
