import QtQuick 2.12
import QtQuick.Layouts 1.11
import QtQuick.Controls 2.12
import QtAVMediaPlayer 1.0
import MediaPlayerBuffer 1.0
import FileUtils 1.0
import CsvParser 0.1
import Thread 0.1
import Multimedia 1.0
import GraphicalEffects 1.0
import QwtQuick2 1.0
import CaptureErrorPlotDataModel 1.0
import CaptureSaturationPlotDataModel 1.0

Column {
    property alias fastForwardButton: fastForwardButton
    property alias rplayButton: rplayButton
    property alias playButton: playButton
    property alias stopButton: stopButton
    property alias rewindButton: rewindButton
    property alias captureButton: captureButton
    property alias decklinkConfigButton: decklinkConfigButton
    property alias deviceNameTextField: deviceNameTextField
    property string outputFilePath: ''
    property alias statusText: statusText.text
    property alias captureFrameInfo: captureFrameInfo
    property alias speedInterpretation: speedInterpretation.source
    property alias playbackBuffer: player.buffer
    property alias player: player
    property var csvParser: csvParser
    property var csvParserUI: csvParserUI
    property alias dataModel: dataModel
    property alias decklinkDataModel: decklinkDataModel

    property int frameSpeed: 0
    property int prev_abst: 0
    property int abst: 0
    property int abst_diff: 0
    property bool isV210: false
    property bool noFrames: true

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
            text: "*** parsing error ***"
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
            text: "Device Connected"
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

        Rectangle {
            height: row.height
            anchors.verticalCenter: row.verticalCenter
            anchors.left: parent.left
            anchors.right: row.left

            Image {
                id: dvcpro_lamp_lit
                fillMode: Image.PreserveAspectFit
                anchors.fill: parent
                horizontalAlignment: Image.AlignLeft
                visible: !noFrames && ((5 - 0.5) < abst_diff && abst_diff < (5 + 0.5))
                source: "/icons/DVCPRO-lamp-lit.svg"
            }

            Image {
                id: dv_lp_lamp_lit
                fillMode: Image.PreserveAspectFit
                anchors.fill: parent
                horizontalAlignment: Image.AlignLeft
                visible: !noFrames && ((6.66 - 0.5) < abst_diff && abst_diff < (6.66 + 0.5))
                source: "/icons/DV-LP-lamp-lit_NEW.svg"
            }

            Image {
                id: dv_sp_lamp_lit
                fillMode: Image.PreserveAspectFit
                anchors.fill: parent
                horizontalAlignment: Image.AlignLeft
                visible: !noFrames && ((10 - 0.5) < abst_diff && abst_diff < (10 + 0.5))
                source: "/icons/DV-SP-lamp-lit_NEW.svg"
            }

            Image {
                id: dvcam_lamp_lit
                fillMode: Image.PreserveAspectFit
                anchors.fill: parent
                horizontalAlignment: Image.AlignLeft
                visible: !noFrames && ((15 - 0.5) < abst_diff && abst_diff < (15 + 0.5))
                source: "/icons/DVCAM-lamp-lit.svg"
            }
        }

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
                        if(noFrames)
                            return "white"

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
        }

        Text {
            id: statusText
            anchors.left: row.right
            anchors.verticalCenter: row.verticalCenter
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
        }

        Button {
            id: decklinkConfigButton
            text: "decklink"
            anchors.right: parent.right
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
                visible: !decklinkConfigButton.visible

                canvasItem.clip: true
                xBottomAxisVisible: true
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
                                return "Of the X frames received, Y contains error concealment (Z within the last 9000 frames shown here)."
                                    .replace("X", dataModel.total).replace("Y", dataModel.totalWithErrors).replace("Z", dataModel.lastWithErrors)
                            } else {
                                return "Of the X frames received, Y contain error concealment (Z within the last 9000 frames shown here)."
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

            QwtQuick2Plot {
                id: decklinkVideoPlot
                Layout.fillWidth: true
                Layout.fillHeight: true
                visible: decklinkConfigButton.visible

                canvasItem.clip: true
                xBottomAxisVisible: true
                yLeftAxisVisible: false
                backgroundColor: "Cornsilk"

                property real yaxis_min: 0
                property real yaxis_high: 725
                property real sat1: 88.7
                property real sat2: 118.2
                property real sat3: 118.2
                property int sat_multiplier: isV210 ? 1 : 4

                yLeftAxisRange: Qt.vector2d(0, yaxis_high)

                Component.onCompleted: {
                    yLeftAxisFont.bold = false
                    yLeftAxisFont.pixelSize = yLeftAxisFont.pixelSize - 2
                    xBottomAxisFont.bold = false
                    xBottomAxisFont.pixelSize = xBottomAxisFont.pixelSize - 2
                }

                QwtQuick2PlotCurve {
                    id: satavgCurve
                    title: "satavg";
                    curveStyle: QwtQuick2PlotCurve.Lines
                    color: "black"
                    titleColor: "darkgray"
                }

                QwtQuick2PlotCurve {
                    id: notInPlayOrRecordSatavgCurve
                    curveStyle: QwtQuick2PlotCurve.Lines
                    color: Qt.rgba(0.1, 0.1, 0.1, 0.5)
                    titleColor: "darkgray"
                }

                QwtQuick2PlotCurve {
                    id: satmaxCurve
                    title: "satmax";
                    curveStyle: QwtQuick2PlotCurve.Lines
                    color: "darkgreen"
                    titleColor: "darkgray"
                }

                QwtQuick2PlotCurve {
                    id: notInPlayOrRecordSatmaxCurve
                    curveStyle: QwtQuick2PlotCurve.Lines
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

                QwtQuick2PlotMarker {
                    lineStyle: QwtQuick2PlotMarker.HLine
                    penStyle: Qt.DashDotDotLine
                    penColor: 'red'
                    value: Qt.point(0, decklinkVideoPlot.sat3 * decklinkVideoPlot.sat_multiplier)
                }

                QwtQuick2PlotMarker {
                    lineStyle: QwtQuick2PlotMarker.HLine
                    penStyle: Qt.DashDotDotLine
                    penColor: 'orange'
                    value: Qt.point(0, decklinkVideoPlot.sat2 * decklinkVideoPlot.sat_multiplier)
                }

                ToolTip {
                    /*
                    text: {
                        if(decklinkDataModel.total < 9000) {
                            return "Of the X frames received, Y contain error concealment."
                                .replace("X", decklinkDataModel.total).replace("Y", decklinkDataModel.totalWithErrors)
                        } else {
                            if(decklinkDataModel.totalWithErrors < 2) {
                                return "Of the X frames received, Y contains error concealment (Z within the last 9000 frames shown here)."
                                    .replace("X", decklinkDataModel.total).replace("Y", decklinkDataModel.totalWithErrors).replace("Z", decklinkDataModel.lastWithErrors)
                            } else {
                                return "Of the X frames received, Y contain error concealment (Z within the last 9000 frames shown here)."
                                    .replace("X", decklinkDataModel.total).replace("Y", decklinkDataModel.totalWithErrors).replace("Z", decklinkDataModel.lastWithErrors)
                            }
                        }
                    }
                    */

                    visible: decklinkTooltipMouseArea.containsMouse
                }

                MouseArea {
                    id: decklinkTooltipMouseArea
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
                decklinkDataModel.update();
            }
        }

        CaptureSaturationPlotDataModel {
            id: decklinkDataModel

            satavgCurve: satavgCurve
            notInPlayOrRecordSatavgCurve: notInPlayOrRecordSatavgCurve

            satmaxCurve: satmaxCurve
            notInPlayOrRecordSatmaxCurve: notInPlayOrRecordSatmaxCurve

            onTotalChanged: {
                console.debug('total changed: ', total)
            }

            onLastChanged: {
                console.debug('last changed: ', last)
            }

            Component.onCompleted: {
                decklinkDataModel.update();
            }
        }
    }
}
