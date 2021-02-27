import QtQuick 2.12
import QtQuick.Layouts 1.11
import QtQuick.Controls 2.12
import FileUtils 1.0

Menu {
    id: recentsPopup

    signal selected(string filePath);
    property var files: []

    onWidthChanged: {
        console.debug('recentsPopup width: ', width)
    }

    Repeater {
        model: files.length
        MenuItem {
            id: menuItem

            TextMetrics {
                font: menuItem.font
                text: files[index]
                onTextChanged: {
                    recentsPopup.width = Math.max(recentsPopup.width, width + spacing + padding)
                }
            }

            text: files[index]

            onClicked: {
                selected(files[index])
            }
        }
    }
}
