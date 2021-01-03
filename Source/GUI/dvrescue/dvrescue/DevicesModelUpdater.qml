import QtQuick 2.0
import QtQuick.Controls 2.12

Item {
    property DevicesModel model;
    property var getDevices;

    Timer {
        id: timer
        repeat: false;
        interval: 1000
        onTriggered: {
            getDevices().then((result) => {
                model.update(result.devices);
                timer.interval = 1000;
                timer.start();
            }).catch((errorResult) => {
                timer.interval = 5000;
                timer.start();
            })
        }
    }

    Component.onCompleted: {
        timer.start();
    }
}
