import QtQuick 2.0
import QtTest 1.0
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import "../../dvrescue"

Item {
    id: root
    width: Math.max(1280, parent ? parent.width : 0)
    height: Math.max(1024, parent ? parent.height : 0)

    ColumnLayout {
        width: root.width
        height: root.height

        Rectangle {
            color: 'white'
            Layout.fillWidth: true
            Layout.minimumHeight: childrenRect.height

            Flow {
                width: parent.width
                property bool needsApply: false;

                Label {
                    text: "Segmenting Rules"
                    font.bold: true
                    verticalAlignment: Text.AlignVCenter
                    height: recordingStartMarkers.implicitHeight
                }

                CheckBox {
                    id: recordingStartMarkers
                    text: "Rec Start Markers"
                    onCheckedChanged: {
                        needsApply = true
                    }
                }
                CheckBox {
                    id: breaksInRecordingTime
                    text: "Rec Time Break"
                    onCheckedChanged: {
                        needsApply = true
                    }
                }
                CheckBox {
                    id: breaksInTimecode
                    text: "Timecode Break"
                    onCheckedChanged: {
                        needsApply = true
                    }
                }
                CheckBox {
                    id: segmentFilesToPreserveAudioSampleRate
                    text: "Audio Setting Change"
                    onCheckedChanged: {
                        needsApply = true
                    }
                }

                Label {
                    text: "Aspect Ratio Change"
                    verticalAlignment: Text.AlignVCenter
                    height: recordingStartMarkers.implicitHeight
                    font.bold: true
                }

                ComboBoxEx {
                    id: aspectRatiosSelector
                    sizeToContents: true
                    model: [
                        "Yes, segment frames by aspect ratio changes",
                        "No and use most common aspect ratio",
                        "No and force segments to use 4/3",
                        "No and force segments to use 16/9"
                    ]
                    onCurrentIndexChanged: {
                        needsApply = true
                    }
                }

                Button {
                    text: "Reset"
                    onClicked: {
                        if(recordingStartMarkers.checked) {
                            recordingStartMarkers.checked = false
                            needsApply = true
                        }

                        if(breaksInRecordingTime.checked) {
                            breaksInRecordingTime.checked = false
                            needsApply = true
                        }

                        if(breaksInTimecode.checked) {
                            breaksInTimecode.checked = false
                            needsApply = true
                        }

                        if(segmentFilesToPreserveAudioSampleRate.checked) {
                            segmentFilesToPreserveAudioSampleRate.checked = false
                            needsApply = true
                        }

                        if(aspectRatiosSelector.currentIndex !== 0) {
                            aspectRatiosSelector.currentIndex = 0
                            needsApply = true
                        }
                    }
                }

                Button {
                    enabled: needsApply
                    text: "Apply"
                }
            }
        }

        Button {
            onClicked: {
                segmentDataView.filterIndex++;
                segmentDataView.invalidateFilter();
            }
        }

        SegmentDataView {
            id: segmentDataView
            Layout.fillWidth: true
            Layout.fillHeight: true

            property int filterIndex: 0
            rowFilter: function(index) {
                console.debug('index: ', index);
                if(index === filterIndex)
                    return false;
            }

            Component.onCompleted: {
                var e = {
                    'Segment #' : '',
                    'Frame #' : '',
                    'Timestamp' : '',
                    'Timecode' : '',
                    'Timecode: Jump/Repeat' : Qt.point(0, 0),
                    'Recording Time' : '',
                    'Recording Time: Jump/Repeat' : Qt.point(0, 0),
                    'Recording Marks' : Qt.point(0, 0),
                    'Video/Audio' : ''
                }

                segmentDataView.model.appendRow(e);
                segmentDataView.model.clear();
            }
        }
    }

    TestCase {
        name: "TestSegmentTable"
        when: windowShown

        SegmentDataParser {
            id: segmentDataParser
        }

        function initTestCase() {
        }

        function test_fill_segment_table() {

            var segment_data = "00:00:00.000000|00:00:00.934267|0|27|00:00:03:02|0|2008-09-28 10:29:53|720x576|25|4:2:0|4/3|32000|4||||-|4032000|-|
    00:00:00.934267|00:00:01.968633|28|58|00:38:01:05|4032000||720x480|30000/1001|4:1:1|4/3|48000|2|||1|-|7752000|-|
    00:00:01.968633|00:00:02.002000|59|59|00:00:00:00|7752000|1970-01-01 00:00:00|720x576|25|4:1:1|4/3|48000|2|||1|-|7896000|-|
    00:00:02.002000|00:00:02.102100|60|62||7896000||720x576|25|4:2:0|4/3|48000|2||||-|8256000|-|
    00:00:02.102100|00:00:02.202200|63|65|00:11:05;12|8256000|2002-12-29 15:15:05|720x480|30000/1001|4:1:1|4/3|48000|2|||1|-|8616000|-|";

            console.debug('test_fill_segment_table...')

            var i = 0;
            segmentDataParser.parse(segment_data, (entry) => {
                                        console.debug('entry: ', JSON.stringify(entry));

                                        var videoAudio = [
                                            entry.frameSize,
                                            entry.frameRate,
                                            entry.chromaSubsampling,
                                            entry.aspectRatio,
                                            entry.samplingRate,
                                            entry.channelCount
                                        ]

                                        ++i
                                        var e = {
                                            'Segment #' : i,
                                            'Frame #' : entry.startFrame,
                                            'Timestamp' : entry.startPts,
                                            'Timecode' : entry.timeCode,
                                            'Timecode: Jump/Repeat' : Qt.point(entry.timeCodeJump, 0),
                                            'Recording Time' : entry.recTimestamp,
                                            'Recording Time: Jump/Repeat' : Qt.point(entry.recTimeJump, 0),
                                            'Recording Marks' : Qt.point(entry.recStart, 0),
                                            'Video/Audio' : videoAudio.join(' ')
                                        }

                                        segmentDataView.model.appendRow(e);
                                    });

            console.debug('...test_fill_segment_table')

            wait(100000)
        }

        function cleanupTestCase() {
        }
    }
}
