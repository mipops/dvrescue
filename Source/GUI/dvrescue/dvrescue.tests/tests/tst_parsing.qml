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
}
