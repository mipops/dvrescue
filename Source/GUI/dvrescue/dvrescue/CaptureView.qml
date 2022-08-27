import QtQuick 2.12
import QtQuick.Layouts 1.11
import QtQuick.Controls 2.12
import QtAVMediaPlayer 1.0
import MediaPlayerBuffer 1.0
import FileWriter 0.1
import CsvParser 0.1
import Thread 0.1
import Multimedia 1.0
import GraphicalEffects 1.0

Column {
    property alias fastForwardButton: fastForwardButton
    property alias rplayButton: rplayButton
    property alias playButton: playButton
    property alias stopButton: stopButton
    property alias rewindButton: rewindButton
    property alias captureButton: captureButton
    property alias deviceNameTextField: deviceNameTextField
    property alias statusText: statusText.text
    property alias captureFrameInfo: captureFrameInfo
    property alias speedInterpretation: speedInterpretation.source
    property alias playbackBuffer: player.buffer
    property alias player: player
    property var fileWriter: fileWriter
    property var csvParser: csvParser
    property var csvParserUI: csvParserUI

    property int frameSpeed: 0
    onFrameSpeedChanged: {
        if(frameSpeed <= -50)
            capturingModeInt = rewing
        if(frameSpeed < 0)
            capturingModeInt = srewing
        if(frameSpeed === 0)
            capturingModeInt = stopped
        if(frameSpeed <= 49)
            capturingModeInt = playing
        else
            capturingModeInt = ffing
    }

    property bool capturing: false;
    onCapturingChanged: {
        if(!capturing)
            frameSpeed = 0
    }

    property string capturingMode: '' // stop
    onCapturingModeChanged: {
        if(capturingMode == 'play')
            capturingModeInt = playing
        else if(capturingMode == 'srew')
            capturingModeInt = srewing
        else if(capturingMode == 'rew')
            capturingModeInt = rewing
        else if(capturingMode == 'ff')
            capturingModeInt = ffing
        else
            capturingModeInt = stopped
    }
    property int capturingModeInt: stopped // stop

    readonly property int stopped: 0
    readonly property int playing: 1
    readonly property int srewing: -1
    readonly property int ffing: 2
    readonly property int rewing: -2

    property bool grabbing: false;
    onGrabbingChanged: {
        if(!grabbing)
            frameSpeed = 0
    }

    playButton.enabled: !grabbing && capturingModeInt !== playing
    rewindButton.enabled: !grabbing && capturingModeInt !== rewing
    rplayButton.enabled: !grabbing && capturingModeInt !== srewing
    fastForwardButton.enabled: !grabbing && capturingModeInt !== ffing
    captureButton.enabled: !grabbing

    Rectangle {
        width: 640
        height: childrenRect.height
        color: "#ffffff"

        TextField {
            id: deviceNameTextField
            width: 640
            readOnly: true
            text: "[0] DV-VCR (Sony GV-D1000)"
            font.family: "Tahoma"
            placeholderText: qsTr("Searching device...")
            font.pointSize: 11
        }
    }

    Rectangle {
        id: image
        width: 640
        height: 480
        color: 'black'

        Text {
            anchors.centerIn: parent
            color: 'white'
            text: "NO SIGNAL"
        }

        VideoOutput {
            id: videoOutput
            anchors.fill: parent
            objectName: "videoOutput"
        }

        MediaPlayerBuffer {
            id: buffer
        }

        Thread {
            id: fileWriterThread
            worker: fileWriter
        }

        FileWriter {
            id: fileWriter
        }

        Thread {
            id: csvWriterThread
            worker: csvParser
        }

        CsvParser {
            id: csvParser
        }

        CsvParser {
            id: csvParserUI
        }

        QtAVMediaPlayer {
            id: player
            videoOutput: videoOutput
            buffer: buffer
            enableAudio: false

            Component.onCompleted: {
                console.debug('MediaPlayer.StoppedState: ', QtAVMediaPlayer.StoppedState);
                console.debug('MediaPlayer.PlayingState: ', QtAVMediaPlayer.PlayingState);
                console.debug('MediaPlayer.PausedState: ', QtAVMediaPlayer.PausedState);

                console.debug('MediaPlayer.NoMedia: ', QtAVMediaPlayer.NoMedia);
                console.debug('MediaPlayer.LoadedMedia: ', QtAVMediaPlayer.LoadedMedia);
                console.debug('MediaPlayer.EndOfMedia: ', QtAVMediaPlayer.EndOfMedia);
                console.debug('MediaPlayer.InvalidMedia: ', QtAVMediaPlayer.InvalidMedia);
            }
        }
    }

    function configure(button, condition) {
        var backgroundItem = button.background
        var children = backgroundItem.children;
        for(var i = 0; i < children.length; ++i) {
            console.debug('child: ', children[i])
            if(children[i].contains("PaddedRectangle")) {
                var paddedRectangle = children[i];
                button.checkable = true;
                button.checked = true;
                var color = paddedRectangle.color;
                paddedRectangle.color = color;
                button.checkable = Qt.binding(() => { return condition() })
                break;
            }
        }
    }

    Rectangle {
        width: 640
        height: childrenRect.height

        Row {
            id: row
            anchors.horizontalCenter: parent.horizontalCenter

            Button {
                id: rewindButton
                icon.color: 'transparent'
                icon.source: "/icons/rewind.svg"
                Component.onCompleted: {
                    configure(this, () => { return !enabled && !grabbing })
                }
            }

            Button {
                id: rplayButton
                icon.color: 'transparent'
                icon.source: "/icons/srew.svg"
                Component.onCompleted: {
                    configure(this, () => { return !enabled && !grabbing })
                }
            }

            Button {
                id: playButton
                icon.color: 'transparent'
                icon.source: "/icons/play.svg"
                Component.onCompleted: {
                    configure(this, () => { return !enabled && !grabbing })
                }
            }

            Button {
                id: stopButton
                icon.color: 'transparent'
                icon.source: "/icons/stop.svg"
            }

            Button {
                id: fastForwardButton
                icon.color: 'transparent'
                icon.source: "/icons/fastforward.svg"
                Component.onCompleted: {
                    configure(this, () => { return !enabled && !grabbing })
                }
            }
            Button {
                id: captureButton
                icon.color: "transparent"
                icon.source: "/icons/capture.svg"
                Component.onCompleted: {
                    configure(this, () => { return !enabled })
                }
            }

            Item {
                width: 15
                height: 50
            }

            Item {
                height: rewindButton.icon.height
                width: rewindButton.icon.width
                anchors.verticalCenter: parent.verticalCenter

                Image {
                    id: speedInterpretation
                    anchors.fill: parent
                    source: {
                        if(frameSpeed <= -50)
                            return rewindButton.icon.source
                        if(frameSpeed < 0)
                            return rplayButton.icon.source
                        if(frameSpeed === 0)
                            return stopButton.icon.source
                        if(frameSpeed <= 49)
                            return playButton.icon.source

                        return fastForwardButton.icon.source
                    }
                }

                ColorOverlay {
                    anchors.fill: speedInterpretation
                    source: speedInterpretation
                    color: {
                        if(frameSpeed <= -50 || frameSpeed >= 50)
                            return "purple"

                        if(frameSpeed <= -33 || frameSpeed >= 33)
                            return "blue"

                        if(frameSpeed <= -31 || frameSpeed >= 31)
                            return "green"

                        if(frameSpeed < 0 || frameSpeed > 0)
                            return "red"

                        return "black"
                    }
                }

                ToolTip {
                    text: "Frame Speed: " + frameSpeed
                    visible: mouseArea.containsMouse
                }

                MouseArea {
                    id: mouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                }
            }
        }

        CaptureFrameInfo {
            id: captureFrameInfo
            anchors.top: row.bottom
            anchors.left: parent.left
            anchors.right: parent.right
        }

        Text {
            id: statusText
            anchors.right: captureFrameInfo.right
            anchors.top: captureFrameInfo.top
            anchors.bottom: captureFrameInfo.bottom
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
        }
    }
}
