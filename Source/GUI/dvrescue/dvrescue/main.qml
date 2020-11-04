import QtQuick 2.15
import QtQuick.Window 2.15
import Launcher 0.1

Window {
    width: 640
    height: 480
    visible: true
    title: qsTr("Hello World")

    AvfCtl {
        id: avfctl
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
        anchors.fill: parent
    }
}
