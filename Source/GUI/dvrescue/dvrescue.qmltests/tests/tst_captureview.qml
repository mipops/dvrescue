import QtQuick 2.0
import QtTest 1.0
import QtQuick.Window 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import "qrc:/"

Rectangle {
    color: 'white'
    width: Math.max(800, parent ? parent.width : 0)
    height: Math.max(800, parent ? parent.height : 0)

    CaptureView {
        id: captureView
        width: parent.width
        height: parent.width
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
            running: true
            interval: 1000
            repeat: true
            onTriggered: {
                captureView.frameSpeed = testcase.frameSpeeds[testcase.index]
                testcase.index = (testcase.index + 1) % testcase.frameSpeeds.length
            }
        }

        function test_captureView() {
            wait(100000)
        }

        function cleanupTestCase() {
        }
    }
}
