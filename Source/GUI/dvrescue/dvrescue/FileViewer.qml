import QtQuick 2.12
import QtQuick.Layouts 1.11
import QtQuick.Controls 2.12
import FileUtils 1.0

Rectangle {
    id: fileViewer
    height: 100
    color: "#ffffff"

    property alias fileView: fileView
    property alias selectedPath: fileView.selectedPath
    property alias files: fileView.files
    property alias updated: fileView.updated

    ColumnLayout {
        anchors.fill: parent

        FileView {
            id: fileView

            Layout.fillHeight: true
            Layout.fillWidth: true
        }
    }
}
