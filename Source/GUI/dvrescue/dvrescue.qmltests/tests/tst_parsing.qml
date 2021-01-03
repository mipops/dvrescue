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
}
