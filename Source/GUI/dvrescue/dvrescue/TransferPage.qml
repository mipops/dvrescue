import QtQuick 2.12
import QtQuick.Layouts 1.11
import QtQuick.Controls 2.12

Rectangle {
    id: rectangle
    color: "#2e3436"
    property alias fastForwardButton: captureView.fastForwardButton
    property alias playButton: captureView.playButton
    property alias stopButton: captureView.stopButton
    property alias rewindButton: captureView.rewindButton
    property alias captureButton: captureView.captureButton
    property alias deviceNameTextField: captureView.deviceNameTextField
    property alias statusText: captureView.statusText
    property alias playbackBuffer: captureView.playbackBuffer
    property alias player: captureView.player
    property alias fileWriter: captureView.fileWriter

    width: 1190
    height: 768

    property bool pendingAction: false;
    property var queryStatusCallback;

    CaptureView {
        id: captureView
        visible: false
    }

    FunkyGridLayout {
        width: parent.width
        height: parent.height
        Repeater {
            id: captureViewRepeater
            model: devicesModel
            delegate: CaptureView {
            }
        }
    }

    /*
    Timer {
        repeat: true
        running: deviceNameTextField.text !== ''
        interval: 100
        property bool querying: false
        onTriggered: {
            if(pendingAction)
                return;

            if(querying === false)
            {
                querying = true;
                queryStatusCallback().then((result) => {
                    console.debug('status: ', JSON.stringify(result.status, 0, 4))
                    if(pendingAction === false) {
                        captureView.statusText = result.status.statusText
                    }
                    querying = false;
                });
            }
        }
    }
    */
}

/*##^##
Designer {
    D{i:0;formeditorZoom:0.9}D{i:24}
}
##^##*/

