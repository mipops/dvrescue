import QtQuick 2.12
import QtQuick.Layouts 1.11
import QtQuick.Controls 2.12

Rectangle {
    id: rectangle
    color: "#2e3436"

    width: 1190
    height: 768

    property var queryStatusCallback;

    FunkyGridLayout {
        width: parent.width
        height: parent.height
        Repeater {
            id: captureViewRepeater
            model: devicesModel
            delegate: CaptureView {
                id: captureView
                property bool pendingAction: false;

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
                            queryStatusCallback(index).then((result) => {
                                console.debug('status: ', JSON.stringify(result.status, 0, 4))
                                if(pendingAction === false) {
                                    captureView.statusText = result.status.statusText
                                }
                                querying = false;
                            }).catch((err) => {
                                querying = false;
                            });
                        }
                    }
                }

                rewindButton.onClicked: {
                    pendingAction = true;
                    statusText = "rewinding..";
                    avfctl.rew(index, (launcher) => {
                       commandsLogs.logCommand(launcher);
                    }).then((result) => {
                       statusText = "rewinding.";
                       pendingAction = false;
                       commandsLogs.logResult(result.outputText);
                       return result;
                    });
                }

                stopButton.onClicked: {
                    pendingAction = true;
                    statusText = "stopping..";
                    avfctl.stop(index, (launcher) => {
                       commandsLogs.logCommand(launcher);
                    }).then((result) => {
                       statusText = "stopping.";
                       pendingAction = false;
                       commandsLogs.logResult(result.outputText);
                       return result;
                    });
                }

                playButton.onClicked: {
                    pendingAction = true;
                    statusText = "playing..";
                    avfctl.play(index, (launcher) => {
                       commandsLogs.logCommand(launcher);
                    }).then((result) => {
                       statusText = "playing.";
                       pendingAction = false;
                       commandsLogs.logResult(result.outputText);
                       return result;
                    });
                }

                fastForwardButton.onClicked: {
                    pendingAction = true;
                    statusText = "fast-forwarding..";
                    avfctl.ff(index,  (launcher) => {
                        commandsLogs.logCommand(launcher);
                    }).then((result) => {
                        statusText = "fast-forwarding.";
                        pendingAction = false;
                        commandsLogs.logResult(result.outputText);
                        return result;
                    });
                }

                captureButton.onClicked: {
                    specifyPathDialog.callback = (fileUrl) => {
                        var filePath = urlToPath(fileUrl);

                        pendingAction = true;
                        player.play()

                        fileWriter.fileName = filePath;
                        fileWriter.open();

                        dvrescue.grab(index, filePath, playbackBuffer, fileWriter, (launcher) => {
                           launcher.errorChanged.connect((errorBytes) => {
                               console.debug('grabbed errorString: ', errorBytes)
                               var errorString = '' + errorBytes;
                               var splitted = errorString.trim().split('\r');
                               statusText = splitted[splitted.length - 1]

                               var values = statusText.split('|')
                               if(values.length === 4) {
                                 captureFrameInfo.frameNumber = values[0];
                                 captureFrameInfo.timeCode = values[1];
                                 captureFrameInfo.recDate = values[2];
                                 captureFrameInfo.recTime = values[3];
                               }
                           });

                           console.debug('logging grab command')
                           commandsLogs.logCommand(launcher);
                        }).then((result) => {
                           commandsLogs.logResult(result.outputText);
                           return result;
                        }).catch((e) => {
                           commandsLogs.logResult(e);
                        });
                    }

                    specifyPathDialog.open();
                }

                deviceNameTextField.text: devicesModel.count === 0 ? '' : devicesModel.get(index).name + " (" + devicesModel.get(index).type + ")"
            }
        }
    }
}

/*##^##
Designer {
    D{i:0;formeditorZoom:0.9}D{i:24}
}
##^##*/

