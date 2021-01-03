import QtQuick 2.0
import QtTest 1.0
import "../../dvrescue"

Item {
    width: 640
    height: 480

    DevicesModel {
        id: model
    }

    AvfCtlParser {
        id: parser
    }

    DevicesModelUpdater {
        model: model
        getDevices: function() {
            return queryDecks();
        }

        property var devicesList1: "2020-11-04 16:02:24.474 avfctl[55693:3849610] Devices:
2020-11-04 16:02:24.474 avfctl[55693:3849610] [0] DV-VCR (Sony GV-D1000)";

        property var devicesList2: "2020-11-04 16:02:24.474 avfctl[55693:3849610] Devices:
2020-11-04 16:02:24.474 avfctl[55693:3849610] [0] DV-VCR (Sony GV-D1000)
2020-11-04 16:02:24.474 avfctl[55693:3849610] [1] DV-VCR (Sony GV-D1000)";

        property var devicesList3: "2020-11-04 16:02:24.474 avfctl[55693:3849610] Devices:
2020-11-04 16:02:24.474 avfctl[55693:3849610] [0] DV-VCR (Sony GV-D1000)
2020-11-04 16:02:24.474 avfctl[55693:3849610] [2] DV-VCR (Sony GV-D1000)
2020-11-04 16:02:24.474 avfctl[55693:3849610] [1] DV-VCR (Sony GV-D1000)";

        property var devicesList4: "2020-11-04 16:02:24.474 avfctl[55693:3849610] Devices:
2020-11-04 16:02:24.474 avfctl[55693:3849610] [2] DV-VCR (Sony GV-D1000)"

        property var devicesLists: [ devicesList1, devicesList2, devicesList3, devicesList4 ]

        function queryDecks() {
            var promise = new Promise((accept, reject) => {

                console.debug('querying decks...');
                Qt.callLater(() => {
                                 try {
                                     var index = Math.round(Math.random() * 2);
                                     var outputText = devicesLists[index];
                                     console.debug('outputText: ', outputText);
                                     accept(parser.parseDevicesList(outputText));
                                 }
                                 catch(err) {
                                     reject(err);
                                 }
                             });
            })

            return promise;
        }
    }

    DevicesView {
        anchors.fill: parent
        model: model
    }

    TestCase {
        name: "TestParsing"
        when: windowShown

        function initTestCase() {
            wait(100000)
        }

        function cleanupTestCase() {
        }
    }
}
