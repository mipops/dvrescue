import QtQuick 2.12
import QtQuick.Layouts 1.11
import QtQuick.Controls 2.12
import FileUtils 1.0

Rectangle {
    id: fileViewer
    height: 100
    color: "#ffffff"

    property alias selectedPath: fileView.selectedPath

    SelectPathDialog {
        id: selectPath
    }

    Column {
        id: buttons
        anchors.right: parent.right
        anchors.top: parent.top

        Button {
            text: qsTr("Add files")
            onClicked: {
                selectPath.callback = (path) => {
                    fileView.add(FileUtils.getFilePath(path));
                }

                selectPath.open();
            }
        }

        Button {
            text: qsTr("Recent")
            onClicked: {

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
