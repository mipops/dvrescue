import QtQuick 2.0
import QtTest 1.0
import "../../dvrescue"

Rectangle {
    id: root
    width: Math.max(640, parent ? parent.width : 0)
    height: Math.max(480, parent ? parent.height : 0)

    DevicesModel {
        id: devicesModel
    }

    DevicesModelUpdater {
        model: devicesModel
        /*
        getDevices: function() {
            return avfctl.queryDecks((launcher) => {
                statusLogs.logCommand(launcher);
            }).then((result) => {
                statusLogs.logResult(result.outputText);
                return result;
            });
        }
        */

        AvfCtlParser {
            id: parser
        }

        getDevices: function() {

            var devicesList1 = "2020-11-04 16:02:24.474 avfctl[55693:3849610] Devices:
2020-11-04 16:02:24.474 avfctl[55693:3849610] [0] DV-VCR (Sony GV-D1000)";

            var devicesList2 = "2020-11-04 16:02:24.474 avfctl[55693:3849610] Devices:
2020-11-04 16:02:24.474 avfctl[55693:3849610] [0] DV-VCR (Sony GV-D1000)
2020-11-04 16:02:24.474 avfctl[55693:3849610] [1] DV-VCR (Sony GV-D1000)";

            var devicesList3 = "2020-11-04 16:02:24.474 avfctl[55693:3849610] Devices:
2020-11-04 16:02:24.474 avfctl[55693:3849610] [0] DV-VCR (Sony GV-D1000)
2020-11-04 16:02:24.474 avfctl[55693:3849610] [2] DV-VCR (Sony GV-D1000)
2020-11-04 16:02:24.474 avfctl[55693:3849610] [1] DV-VCR (Sony GV-D1000)";

            var devicesList4 = "2020-11-04 16:02:24.474 avfctl[55693:3849610] Devices:
2020-11-04 16:02:24.474 avfctl[55693:3849610] [0] DV-VCR (Sony GV-D1000)
2020-11-04 16:02:24.474 avfctl[55693:3849610] [1] DV-VCR (Sony GV-D1000)
2020-11-04 16:02:24.474 avfctl[55693:3849610] [2] DV-VCR (Sony GV-D1000)
2020-11-04 16:02:24.474 avfctl[55693:3849610] [3] DV-VCR (Sony GV-D1000)";

            var devicesList5 = "2020-11-04 16:02:24.474 avfctl[55693:3849610] Devices:
2020-11-04 16:02:24.474 avfctl[55693:3849610] [0] DV-VCR (Sony GV-D1000)
2020-11-04 16:02:24.474 avfctl[55693:3849610] [1] DV-VCR (Sony GV-D1000)
2020-11-04 16:02:24.474 avfctl[55693:3849610] [2] DV-VCR (Sony GV-D1000)
2020-11-04 16:02:24.474 avfctl[55693:3849610] [3] DV-VCR (Sony GV-D1000)
2020-11-04 16:02:24.474 avfctl[55693:3849610] [4] DV-VCR (Sony GV-D1000)";

            var devicesList6 = "2020-11-04 16:02:24.474 avfctl[55693:3849610] Devices:
2020-11-04 16:02:24.474 avfctl[55693:3849610] [0] DV-VCR (Sony GV-D1000)
2020-11-04 16:02:24.474 avfctl[55693:3849610] [1] DV-VCR (Sony GV-D1000)
2020-11-04 16:02:24.474 avfctl[55693:3849610] [2] DV-VCR (Sony GV-D1000)
2020-11-04 16:02:24.474 avfctl[55693:3849610] [3] DV-VCR (Sony GV-D1000)
2020-11-04 16:02:24.474 avfctl[55693:3849610] [4] DV-VCR (Sony GV-D1000)
2020-11-04 16:02:24.474 avfctl[55693:3849610] [5] DV-VCR (Sony GV-D1000)";

            var devicesList7 = "2020-11-04 16:02:24.474 avfctl[55693:3849610] Devices:
2020-11-04 16:02:24.474 avfctl[55693:3849610] [0] DV-VCR (Sony GV-D1000)
2020-11-04 16:02:24.474 avfctl[55693:3849610] [1] DV-VCR (Sony GV-D1000)
2020-11-04 16:02:24.474 avfctl[55693:3849610] [2] DV-VCR (Sony GV-D1000)
2020-11-04 16:02:24.474 avfctl[55693:3849610] [3] DV-VCR (Sony GV-D1000)
2020-11-04 16:02:24.474 avfctl[55693:3849610] [4] DV-VCR (Sony GV-D1000)
2020-11-04 16:02:24.474 avfctl[55693:3849610] [5] DV-VCR (Sony GV-D1000)
2020-11-04 16:02:24.474 avfctl[55693:3849610] [6] DV-VCR (Sony GV-D1000)";

            var devicesList8 = "2020-11-04 16:02:24.474 avfctl[55693:3849610] Devices:
2020-11-04 16:02:24.474 avfctl[55693:3849610] [0] DV-VCR (Sony GV-D1000)
2020-11-04 16:02:24.474 avfctl[55693:3849610] [1] DV-VCR (Sony GV-D1000)
2020-11-04 16:02:24.474 avfctl[55693:3849610] [2] DV-VCR (Sony GV-D1000)
2020-11-04 16:02:24.474 avfctl[55693:3849610] [3] DV-VCR (Sony GV-D1000)
2020-11-04 16:02:24.474 avfctl[55693:3849610] [4] DV-VCR (Sony GV-D1000)
2020-11-04 16:02:24.474 avfctl[55693:3849610] [5] DV-VCR (Sony GV-D1000)
2020-11-04 16:02:24.474 avfctl[55693:3849610] [6] DV-VCR (Sony GV-D1000)
2020-11-04 16:02:24.474 avfctl[55693:3849610] [7] DV-VCR (Sony GV-D1000)";

            var devicesLists = [
                devicesList1, devicesList2, devicesList3, devicesList4,
                devicesList5, devicesList6, devicesList7, devicesList8
            ]

            var promise = new Promise((accept, reject) => {

                console.debug('querying decks...');
                Qt.callLater(() => {
                                 try {
                                     var index = Math.round(Math.random() * devicesLists.length);
                                     var outputText = devicesLists[2];
                                     console.debug('outputText: ', outputText);
                                     accept({ devices: parser.parseDevicesList(outputText) });
                                 }
                                 catch(err) {
                                     reject(err);
                                 }
                             });
            })

            return promise;
        }
    }

    FunkyGridLayout {
        width: root.width
        height: root.height
        Repeater {
            id: captureViewRepeater
            model: devicesModel
            delegate: CaptureView {
                id: captureView

                Component.onCompleted: {
                    console.debug('CaptureView created')
                }

                Component.onDestruction: {
                    console.debug('CaptureView destroyed')
                }
            }
        }
    }

    TestCase {
        name: "TestDevicesList"
        when: windowShown

        function initTestCase() {
        }

        function test_1() {
            wait(100000)
        }

        function cleanupTestCase() {
        }
    }
}
