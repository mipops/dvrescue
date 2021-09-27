import QtQuick 2.0
import QtTest 1.0
import "../../dvrescue"

Item {
    id: root
    width: Math.max(1280, parent ? parent.width : 0)
    height: Math.max(1024, parent ? parent.height : 0)

    SegmentDataView {
        id: segmentDataView
        width: root.width
        height: root.height

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
