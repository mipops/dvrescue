import QtQuick 2.12
import QtQuick.Layouts 1.11
import QtQuick.Controls 2.12
import ConnectionUtils 1.0

Rectangle {
    id: rectangle
    color: "#2e3436"

    width: 1190
    height: 768

    signal grabCompleted(string filePath);

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
                            dvrescue.status(index).then((result) => {
                                console.debug('status: ', result.status)
                                if(pendingAction === false) {
                                    captureView.statusText = result.status
                                }
                                querying = false;
                            }).catch((err) => {
                                querying = false;
                            });
                        }
                    }
                }

                property int indexOfFramePos: -1;
                property int indexOfTimecode: -1;
                property int indexOfRecDateTime: -1;
                property int indexOfSourceSpeed: -1;
                property int indexOfFrameSpeed: -1;
                property int indexOfBlockErrors: -1;
                property int indexOfBlockErrorsEven: -1;

                function onColumnsChanged(columns) {

                    // ["FramePos","abst","abst_r","abst_nc","tc","tc_r","tc_nc","rdt","rdt_r","rdt_nc","rec_start","rec_end","Used","Status","Comments","BlockErrors","BlockErrors_Even","IssueFixed","SourceSpeed","FrameSpeed","InputPos","OutputPos"]
                    var columnNames = [];

                    columnNames = columns
                    console.debug('columnNames: ', JSON.stringify(columnNames))

                    indexOfFramePos = columnNames.indexOf('FramePos');
                    indexOfTimecode = columnNames.indexOf('tc');
                    indexOfRecDateTime = columnNames.indexOf('rdt');
                    indexOfSourceSpeed = columnNames.indexOf('SourceSpeed');
                    indexOfFrameSpeed = columnNames.indexOf('FrameSpeed');
                    indexOfBlockErrors = columnNames.indexOf('BlockErrors');
                    indexOfBlockErrorsEven = columnNames.indexOf('BlockErrors_Even');

                    console.debug('indexOfFramePos: ', indexOfFramePos)
                    console.debug('indexOfTimecode: ', indexOfTimecode)
                    console.debug('indexOfRecDateTime: ', indexOfRecDateTime)
                    console.debug('indexOfSourceSpeed: ', indexOfSourceSpeed)
                    console.debug('indexOfFrameSpeed: ', indexOfFrameSpeed)
                    console.debug('indexOfBlockErrors: ', indexOfBlockErrors)
                    console.debug('indexOfBlockErrorsEven: ', indexOfBlockErrorsEven)
                }

                function onEntriesReceived(entries) {
                    var framePos = 0;
                    if(indexOfFramePos !== -1) {
                        framePos = captureFrameInfo.frameNumber = entries[indexOfFramePos]
                    }

                    if(indexOfTimecode !== -1) {
                        captureFrameInfo.timeCode = entries[indexOfTimecode]
                    }

                    if(indexOfRecDateTime !== -1) {
                        var rdt = entries[indexOfRecDateTime];
                        captureFrameInfo.recTime = rdt;
                    }

                    /*
                    if(indexOfSourceSpeed !== -1) {
                        var sourceSpeed = entries[indexOfSourceSpeed]
                        speedValueText = sourceSpeed
                    }
                    */

                    if(indexOfFrameSpeed !== -1) {
                        var frameSpeedValue = entries[indexOfFrameSpeed]
                        if(frameSpeedValue)
                            frameSpeed = frameSpeedValue
                    }

                    if(indexOfBlockErrors !== -1 && indexOfBlockErrorsEven !== -1) {
                        var blockErrors = entries[indexOfBlockErrors]
                        var blockErrorsEven = entries[indexOfBlockErrorsEven]

                        dataModel.append(framePos, blockErrorsEven, blockErrors, captureView.capturingModeInt == captureView.playing)
                    }
                }

                function doCapture(captureCmd) {
                    csvParser.columnsChanged.disconnect(onColumnsChanged);
                    csvParserUI.entriesReceived.disconnect(onEntriesReceived);
                    ConnectionUtils.disconnect(csvParser, 'entriesReceived(const QStringList&)')
                    playbackBuffer.clear();

                    capturing = true;
                    pendingAction = true;
                    player.play()

                    statusText = "capturing..";
                    capturingMode = captureCmd;

                    dvrescue.capture(index, playbackBuffer, csvParser, captureCmd, (launcher) => {
                       csvParser.columnsChanged.connect(onColumnsChanged);
                       var result = ConnectionUtils.connectToSignalQueued(csvParser, 'entriesReceived(const QStringList&)', csvParserUI, 'entriesReceived(const QStringList&)');
                       csvParserUI.entriesReceived.connect(onEntriesReceived);

                       console.debug('logging start capture command')
                       commandsLogs.logCommand(launcher);
                    }).then((result) => {
                        capturing = false;
                        capturingMode = '';
                        pendingAction = false;
                        player.stop();
                        commandsLogs.logResult(result.outputText);
                        return result;
                    }).catch((e) => {
                        capturing = false;
                        capturingMode = '';
                        pendingAction = false
                        player.stop();
                        commandsLogs.logResult(e);
                    });
                }

                function doDeckControl(deckControlCmd, deckControlStatus) {
                    pendingAction = true;
                    capturingMode = deckControlCmd;

                    statusText = deckControlStatus + "..";
                    dvrescue.control(index, deckControlCmd, (launcher) => {
                        commandsLogs.logCommand(launcher);
                    }).then((result) => {
                        statusText = deckControlCmd + ".";
                        pendingAction = false;
                        commandsLogs.logResult(result.outputText);
                        return result;
                    });
                }

                rewindButton.onClicked: {
                    if(!capturing)
                        doCapture('rew')
                    else
                        doDeckControl('rew', 'rewinding')
                }

                stopButton.onClicked: {
                    doDeckControl('stop', 'stopping')
                }

                rplayButton.onClicked: {
                    if(!capturing)
                        doCapture('srew')
                    else
                        doDeckControl('srew', 'rplaying')
                }

                playButton.onClicked: {
                    if(!capturing)
                        doCapture('play')
                    else
                        doDeckControl('play', 'playing')
                }

                fastForwardButton.onClicked: {
                    if(!capturing)
                        doCapture('ff')
                    else
                        doDeckControl('ff', 'fast-forwarding')
                }

                captureButton.onClicked: {
                    specifyPathDialog.callback = (fileUrl) => {
                        csvParser.columnsChanged.disconnect(onColumnsChanged);
                        csvParserUI.entriesReceived.disconnect(onEntriesReceived);
                        ConnectionUtils.disconnect(csvParser, 'entriesReceived(const QStringList&)')
                        playbackBuffer.clear();

                        var filePath = urlToPath(fileUrl);

                        pendingAction = true;
                        player.play()

                        fileWriter.fileName = filePath;
                        fileWriter.open();

                        var columnNames = [];
                        var indexOfFramePos = -1;
                        var indexOfTimecode = -1;
                        var indexOfRecDateTime = -1;

                        grabbing = true;
                        dvrescue.grab(index, filePath, playbackBuffer, fileWriter, csvParser, (launcher) => {
                           outputFilePath = filePath
                           csvParser.columnsChanged.connect(onColumnsChanged);
                           var result = ConnectionUtils.connectToSignalQueued(csvParser, 'entriesReceived(const QStringList&)', csvParserUI, 'entriesReceived(const QStringList&)');
                           csvParserUI.entriesReceived.connect(onEntriesReceived);

                           console.debug('logging grab command')
                           commandsLogs.logCommand(launcher);
                        }).then((result) => {
                           grabbing = false;
                           pendingAction = false;
                           player.stop();
                           commandsLogs.logResult(result.outputText);
                           grabCompleted(filePath)
                           return result;
                        }).catch((e) => {
                           grabbing = false;
                           pendingAction = false;
                           player.stop();
                           commandsLogs.logResult(e);
                        });
                    }

                    if(capturing) {
                        dvrescue.control(index, 'stop', (launcher) => {
                           commandsLogs.logCommand(launcher);
                        }).then((result) => {
                           statusText = "stopping.";
                           commandsLogs.logResult(result.outputText);

                           specifyPathDialog.reset();
                           specifyPathDialog.open();
                        });
                    } else {
                        specifyPathDialog.reset();
                        specifyPathDialog.open();
                    }
                }

                deviceNameTextField.text: devicesModel.count === 0 ? '' : devicesModel.get(index).name + " (" + devicesModel.get(index).type + ")"
            }
        }
    }

    AnimatedImage {
        anchors.centerIn: parent
        source: "/icons/no_decks.gif"
        playing: devicesModel.count === 0
        visible: devicesModel.count === 0
    }
}

/*##^##
Designer {
    D{i:0;formeditorZoom:0.9}D{i:24}
}
##^##*/

