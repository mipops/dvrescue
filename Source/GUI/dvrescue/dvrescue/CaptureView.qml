import QtQuick 2.12
import QtQuick.Layouts 1.11
import QtQuick.Controls 2.12
import QtAVMediaPlayer 1.0
import MediaPlayerBuffer 1.0
import FileWriter 0.1
import CsvParser 0.1
import Thread 0.1
import Multimedia 1.0

Column {
    property alias fastForwardButton: fastForwardButton
    property alias playButton: playButton
    property alias stopButton: stopButton
    property alias rewindButton: rewindButton
    property alias captureButton: captureButton
    property alias deviceNameTextField: deviceNameTextField
    property alias statusText: statusText.text
    property alias captureFrameInfo: captureFrameInfo
    property alias playbackBuffer: player.buffer
    property alias player: player
    property var fileWriter: fileWriter
    property var csvParser: csvParser
    property var csvParserUI: csvParserUI

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
            }

            Button {
                id: playButton
                icon.color: 'transparent'
                icon.source: "/icons/play.svg"
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
            }
            Button {
                id: captureButton
                icon.color: "transparent"
                icon.source: "/icons/capture.svg"
            }
        }

        Text {
            id: statusText
            visible: false
            anchors.left: row.right
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
        }

        CaptureFrameInfo {
            id: captureFrameInfo
            anchors.top: row.bottom
            anchors.left: parent.left
            anchors.right: parent.right
        }
    }
}
