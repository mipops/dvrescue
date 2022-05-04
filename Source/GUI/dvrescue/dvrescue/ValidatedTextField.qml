import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import Qt.labs.settings 1.0
import Launcher 0.1
import FileUtils 1.0
import SettingsUtils 1.0
import QwtQuick2 1.0

TextField {

    property var validate

    onTextChanged: {
        color = 'black'
        timer.start()
    }

    Timer {
        id: timer
        repeat: false
        interval: 500

        onTriggered: {
            if(validate) {
                if(validate(text)) {
                    color = 'green'
                } else {
                    color = 'red'
                }
            }
        }
    }
}
