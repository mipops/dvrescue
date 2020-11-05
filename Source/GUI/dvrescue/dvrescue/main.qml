import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import Qt.labs.settings 1.0
import Launcher 0.1

Window {
    width: 640
    height: 480
    visible: true
    title: qsTr("Hello World")

    AvfCtl {
        id: avfctl
    }

    Settings {
        property alias avfctlCmd: avfctl.avfctlCmd
    }

    RowLayout {
        id: selectAvfctlRow
        anchors.left: parent.left
        anchors.right: parent.right

        TextField {
            id: input

            Layout.fillWidth: true
            onAccepted: {
                avfctl.avfctlCmd = text
            }

            Component.onCompleted: {
                input.text = avfctl.avfctlCmd
            }
        }

        Button {
            text: "Apply"
            onClicked: {
                avfctl.avfctlCmd = input.text
            }
        }
    }

    DevicesModel {
        id: devicesModel
    }

    DevicesModelUpdater {
        model: devicesModel
        getDevices: function() {
            return avfctl.queryDecks();
        }
    }

    DevicesView {
        model: devicesModel
        anchors.top: selectAvfctlRow.bottom
    }
}
