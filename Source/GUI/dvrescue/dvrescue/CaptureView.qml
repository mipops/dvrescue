import QtQuick 2.12
import QtQuick.Layouts 1.11
import QtQuick.Controls 2.12
import QtAVMediaPlayer 1.0
import MediaPlayerBuffer 1.0
import FileUtils 1.0
import FileWriter 0.1
import CsvParser 0.1
import Thread 0.1
import Multimedia 1.0
import GraphicalEffects 1.0
import QwtQuick2 1.0
import CaptureErrorPlotDataModel 1.0

Column {
    property alias fastForwardButton: fastForwardButton
    property alias rplayButton: rplayButton
    property alias playButton: playButton
    property alias stopButton: stopButton
    property alias rewindButton: rewindButton
    property alias captureButton: captureButton
    property alias deviceNameTextField: deviceNameTextField
    property string outputFilePath: ''
    property alias statusText: statusText.text
    property alias captureFrameInfo: captureFrameInfo
    property alias speedInterpretation: speedInterpretation.source
    property alias playbackBuffer: player.buffer
    property alias player: player
    property var fileWriter: fileWriter
    property var csvParser: csvParser
    property var csvParserUI: csvParserUI
    property alias dataModel: dataModel

    property int frameSpeed: 0

    readonly property int thresholdBetweenFastPlayAndPlay: 35
    readonly property int thresholdBetweenPlayAndSlowPlay: 30

    onFrameSpeedChanged: {
        if(frameSpeed < 0) {
            if(frameSpeed <= -thresholdBetweenFastPlayAndPlay)
                capturingModeInt = rewing
            else
                capturingModeInt = srewing
        } else if(frameSpeed > 0) {
            if(frameSpeed >= thresholdBetweenFastPlayAndPlay)
                capturingModeInt = ffing
            else
                capturingModeInt = playing
        } else {
            capturingModeInt = stopped
        }
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

        console.debug('capturingModeInt => ', capturingModeInt)
    }
    property int capturingModeInt: stopped // stop

    readonly property int stopped: 0
    readonly property int playing: 1
    readonly property int srewing: -1
    readonly property int ffing: 2
    readonly property int rewing: -2

    property bool grabbing: false;
    onGrabbingChanged: {
        if(!grabbing) {
            frameSpeed = 0
            outputFilePath = ''
        }
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
            width: parent.width - outputFileNameTextField.width
            readOnly: true
            text: "[0] DV-VCR (Sony GV-D1000)"
            font.family: "Tahoma"
            placeholderText: qsTr("Searching device...")
            font.pointSize: 11
        }

        TextField {
            id: outputFileNameTextField
            horizontalAlignment: "AlignRight"
            anchors.right: parent.right
            readOnly: true
            width: contentWidth
            text: FileUtils.getFileName(outputFilePath)
            font.pointSize: 11

            MouseArea {
                acceptedButtons: Qt.RightButton
                anchors.fill: parent
                onClicked: {
                    Qt.openUrlExternally('file:///' + FileUtils.getFileDir(outputFilePath))
                }
            }
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
            var child = '' + children[i]
            console.debug('child: ', child, typeof(child))
            if(child.includes("PaddedRectangle")) {
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
                onClicked: {
                    if(grabbing)
                        dataModel.reset()
                }

                Component.onCompleted: {
                    configure(this, () => { return !enabled && !grabbing })
                }
            }

            Button {
                id: rplayButton
                icon.color: 'transparent'
                icon.source: "/icons/srew.svg"
                onClicked: {
                    if(grabbing)
                        dataModel.reset()
                }

                Component.onCompleted: {
                    configure(this, () => { return !enabled && !grabbing })
                }
            }

            Button {
                id: playButton
                icon.color: 'transparent'
                icon.source: "/icons/play.svg"
                onClicked: {
                    if(grabbing)
                        dataModel.reset()
                }

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
                onClicked: {
                    if(grabbing)
                        dataModel.reset()
                }

                Component.onCompleted: {
                    configure(this, () => { return !enabled && !grabbing })
                }
            }
            Button {
                id: captureButton
                icon.color: "transparent"
                icon.source: "/icons/capture.svg"
                onClicked: {
                    if(grabbing)
                        dataModel.reset()
                }

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
                        switch(capturingModeInt) {
                        case rewing:
                            return rewindButton.icon.source
                        case srewing:
                            return rplayButton.icon.source
                        case playing:
                            return playButton.icon.source
                        case ffing:
                            return fastForwardButton.icon.source
                        default:
                            return stopButton.icon.source
                        }
                    }
                }

                ColorOverlay {
                    anchors.fill: speedInterpretation
                    source: speedInterpretation
                    color: {
                        if(frameSpeed <= -thresholdBetweenFastPlayAndPlay || frameSpeed >= thresholdBetweenFastPlayAndPlay)
                            return "purple"

                        if(frameSpeed <= -thresholdBetweenPlayAndSlowPlay || frameSpeed >= thresholdBetweenPlayAndSlowPlay)
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

            Item {
                width: 15
                height: 50
            }

            Text {
                id: statusText
                anchors.verticalCenter: parent.verticalCenter
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
            }
        }

        CaptureFrameInfo {
            id: captureFrameInfo
            anchors.top: row.bottom
            anchors.left: parent.left
            anchors.right: parent.right
        }

        RowLayout {
            anchors.top: captureFrameInfo.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            height: 80

            QwtQuick2Plot {
                id: videoPlot
                Layout.fillWidth: true
                Layout.fillHeight: true

                canvasItem.clip: true
                xBottomAxisVisible: false
                yLeftAxisVisible: false
                backgroundColor: "Cornsilk"
                yLeftAxisRange: Qt.vector2d(-900, 900)

                Component.onCompleted: {
                    yLeftAxisFont.bold = false
                    yLeftAxisFont.pixelSize = yLeftAxisFont.pixelSize - 2
                    xBottomAxisFont.bold = false
                    xBottomAxisFont.pixelSize = xBottomAxisFont.pixelSize - 2
                }

                QwtQuick2PlotCurve {
                    id: evenCurve
                    title: "even";
                    curveStyle: QwtQuick2PlotCurve.Sticks
                    color: "darkgreen"
                    titleColor: "darkgray"
                }

                QwtQuick2PlotCurve {
                    id: oddCurve
                    title: "odd";
                    curveStyle: QwtQuick2PlotCurve.Sticks
                    color: "green"
                    titleColor: "darkgray"
                }

                QwtQuick2PlotCurve {
                    id: notInPlayOrRecordCurveEven
                    curveStyle: QwtQuick2PlotCurve.Sticks
                    color: Qt.rgba(0.1, 0.1, 0.1, 0.5)
                    titleColor: "darkgray"
                }

                QwtQuick2PlotCurve {
                    id: notInPlayOrRecordCurveOdd
                    curveStyle: QwtQuick2PlotCurve.Sticks
                    color: Qt.rgba(0.1, 0.1, 0.1, 0.5)
                    titleColor: "darkgray"
                }

                QwtQuick2PlotGrid {
                    enableXMin: true
                    enableYMin: true
                    majorPenColor: 'darkGray'
                    majorPenStyle: Qt.DotLine
                    minorPenColor: 'transparent'
                    minorPenStyle: Qt.DotLine
                }

                ToolTip {
                    text: {
                        if(dataModel.total < 9000) {
                            return "Of the X frames received, Y contain error concealment."
                                .replace("X", dataModel.total).replace("Y", dataModel.totalWithErrors)
                        } else {
                            if(dataModel.totalWithErrors < 2) {
                                return "Of the X frames received, Y contains error concealment (Z within the last 9000 frames)."
                                    .replace("X", dataModel.total).replace("Y", dataModel.totalWithErrors).replace("Z", dataModel.lastWithErrors)
                            } else {
                                return "Of the X frames received, Y contain error concealment (Z within the last 9000 frames)."
                                    .replace("X", dataModel.total).replace("Y", dataModel.totalWithErrors).replace("Z", dataModel.lastWithErrors)
                            }
                        }
                    }

                    visible: tooltipMouseArea.containsMouse
                }

                MouseArea {
                    id: tooltipMouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                }
            }
        }

        CaptureErrorPlotDataModel {
            id: dataModel
            evenCurve: evenCurve
            oddCurve: oddCurve
            notInPlayOrRecordCurveEven: notInPlayOrRecordCurveEven
            notInPlayOrRecordCurveOdd: notInPlayOrRecordCurveOdd

            onTotalChanged: {
                console.debug('total changed: ', total)
            }

            onTotalWithErrorsChanged: {
                console.debug('total with errors changed: ', totalWithErrors)
            }

            onLastChanged: {
                console.debug('last changed: ', last)
            }

            onLastWithErrorsChanged: {
                console.debug('last with errors changed: ', lastWithErrors)
            }

            Component.onCompleted: {
                dataModel.update();
            }
        }

        Timer {
            id: refreshTimer
            interval: 500
            running: true
            repeat: true
            onTriggered: {
                console.debug('updating plots...')
                dataModel.update();
            }
        }
    }
}
