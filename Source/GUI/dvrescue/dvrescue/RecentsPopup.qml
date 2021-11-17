import QtQuick 2.12
import QtQuick.Layouts 1.11
import QtQuick.Controls 2.12
import FileUtils 1.0
import Clipboard 1.0

Menu {
    id: recentsPopup

    signal selected(string filePath);
    property alias filesModel: filesRepeater.model

    Repeater {
        id: filesRepeater
        MenuItem {
            id: menuItem

            TextMetrics {
                font: menuItem.font
                text: filePath
                onTextChanged: {
                    recentsPopup.width = Math.max(recentsPopup.width, width + spacing + padding)
                }
            }

            text: filePath

            onClicked: {
                recentsPopup.close();
                selected(filePath)
            }

            onPressAndHold: {
                Clipboard.setText(FileUtils.getFilePath(filePath, true))
            }
        }
    }
}
