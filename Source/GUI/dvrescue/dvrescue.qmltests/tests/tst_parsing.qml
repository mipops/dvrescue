import QtQuick 2.0
import QtTest 1.0
import "../../dvrescue"

TestCase {
    name: "TestParsing"
    when: windowShown

    AvfCtlParser {
        id: parser
    }

    function initTestCase() {
    }

    function cleanupTestCase() {
    }

    function test_parsing() {
        var devicesList = "2020-11-04 16:02:24.474 avfctl[55693:3849610] Devices:
2020-11-04 16:02:24.474 avfctl[55693:3849610] [0] DV-VCR (Sony GV-D1000)";

        parser.parseDevicesList(devicesList);
    }

    function test_play_output_parsing() {
        var playOutput = "2020-11-06 05:40:25.509 avfctl[23782:7704522] Mode changed: 0 -> 1
2020-11-06 05:40:25.510 avfctl[23782:7704522] Speed changed: 0.000000 -> 2.000000";

        var output = parser.parseStateChanged(playOutput);

        console.debug('mode: ', output.mode.from, output.mode.to)
        console.debug('speed: ', output.speed.from, output.speed.to)
    }

    function test_status_parsing() {
        var statusOutput = "2021-08-27 13:33:24.348 avfctl[99766:4607139] Device [0] DV-VCR (Sony HVR-M15AU) status: stopped\n"

        var output = parser.parseStatus(statusOutput);
        console.debug('status: ', output.status);
    }

    SegmentDataParser {
        id: segmentDataParser
    }

    function test_segment_data_parsing() {
        var segment_data = "00:00:00.000000|00:00:00.934267|0|27|00:00:03:02|0|2008-09-28 10:29:53|720x576|25|4:2:0|4/3|32000|4||||-|4032000|-|
00:00:00.934267|00:00:01.968633|28|58|00:38:01:05|4032000||720x480|30000/1001|4:1:1|4/3|48000|2|||1|-|7752000|-|
00:00:01.968633|00:00:02.002000|59|59|00:00:00:00|7752000|1970-01-01 00:00:00|720x576|25|4:1:1|4/3|48000|2|||1|-|7896000|-|
00:00:02.002000|00:00:02.102100|60|62||7896000||720x576|25|4:2:0|4/3|48000|2||||-|8256000|-|
00:00:02.102100|00:00:02.202200|63|65|00:11:05;12|8256000|2002-12-29 15:15:05|720x480|30000/1001|4:1:1|4/3|48000|2|||1|-|8616000|-|";

        var entries = [];
        var columnsCount = 0;

        segmentDataParser.parse(segment_data, (entry, columns) => {
                                    if(columnsCount == 0)
                                        columnsCount = columns.length;

                                    verify(columnsCount === columns.length)
                                });
    }
}
