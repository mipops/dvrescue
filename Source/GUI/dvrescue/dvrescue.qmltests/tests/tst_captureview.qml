import QtQuick 2.0
import QtTest 1.0
import QtQuick.Window 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import "qrc:/"

Rectangle {
    color: 'white'
    width: Math.max(1200, parent ? parent.width : 0)
    height: Math.max(800, parent ? parent.height : 0)

    DevicesModel {
        id: devicesModel
    }

    CaptureView {
        id: singleCaptureView
        visible: false
        width: parent.width
        height: parent.width
    }

    CapturePage {
        id: capturePage
        visible: false;
    }

    DecklinkConfigPopup {
        id: decklinkConfigPopup
        controlsModel: [{name : 'No control', id: ''}, {name: 'Test', id: '0'}]

        onRejected: {
            console.debug('cancelled');

            console.debug('currentControlIndex =', currentControlIndex);
            console.debug('currentVideoModeIndex =', currentVideoModeIndex);
            console.debug('currentVideoSourceIndex =', currentVideoSourceIndex);
            console.debug('currentAudioSourceIndex =', currentAudioSourceIndex);
            console.debug('currentTimecodesIndex =', currentTimecodesIndex);
        }

        onAccepted: {
            console.debug('accepted');

            console.debug('currentControlIndex =', currentControlIndex);
            console.debug('currentVideoModeIndex =', currentVideoModeIndex);
            console.debug('currentVideoSourceIndex =', currentVideoSourceIndex);
            console.debug('currentAudioSourceIndex =', currentAudioSourceIndex);
            console.debug('currentTimecodesIndex =', currentTimecodesIndex);
        }
    }

    TestCase {
        name: "TestCaptureView"
        when: windowShown
        id: testcase

        function initTestCase() {
        }

        property var frameSpeeds: [-50,
            -49, -33,
            -32, -31,
            -30, -1,
            0,
            1, 30,
            31, 32,
            33, 49,
            50
        ]

        property int index: 0

        Timer {
            id: frameSpeedTimer
            running: false
            interval: 1000
            repeat: true
            onTriggered: {
                captureView.frameSpeed = testcase.frameSpeeds[testcase.index]
                testcase.index = (testcase.index + 1) % testcase.frameSpeeds.length
            }
        }

        Timer {
            id: chartsTimer
            property int frameNumber: 0

            running: false
            interval: 100
            repeat: true
            onTriggered: {
                captureView.dataModel.append(frameNumber, frameNumber, - frameNumber / 2, true)
                ++frameNumber;
                if(frameNumber == 900)
                    frameNumber = 0;

                console.debug('frameNumber: ', frameNumber)
            }
        }

        function test_captureView_dynamic() {
            singleCaptureView.visible = true;
            frameSpeedTimer.running = true;
            chartsTimer.running = true;
            wait(100000)
        }

        function test_captureView() {
            var devices = [];
            devices.push({index: 0, name: 'test', type: 'dv'})

            var controls = JSON.parse('[
    {
        "id": "/dev/tty.wlan-debug",
        "name": "tty.wlan-debug"
    },
    {
        "id": "/dev/tty.RS422 Deck Control",
        "name": "tty.RS422 Deck Control"
    },
    {
        "id": "/dev/tty.Bluetooth-Incoming-Port",
        "name": "tty.Bluetooth-Incoming-Port"
    },
    {
        "id": "/dev/tty.usbserial-A10KXF88",
        "name": "tty.usbserial-A10KXF88"
    }
]')

            devices.push({index: 1, name: 'test', type: 'DeckLink', controls: controls})

            devices.forEach((device) => {
                                console.debug('adding ', device.id, device.name, device.type, JSON.stringify(device.controls ? device.controls : []));
                                devicesModel.append({index: device.id, name: device.name, type: device.type, controls: device.controls ? JSON.stringify(device.controls) : ''})
                            })

            capturePage.visible = true;
            wait(100000)
        }

        function test_decklinkConfigPopup() {
            decklinkConfigPopup.open();

            wait(100000)
        }

        function cleanupTestCase() {
        }
    }
}
