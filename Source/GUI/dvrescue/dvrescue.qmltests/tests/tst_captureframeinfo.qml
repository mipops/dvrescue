import QtQuick 2.0
import QtTest 1.0
import QtQuick.Window 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 1.4 as QQC1
import "../../dvrescue"

Rectangle {
    color: 'white'
    width: Math.max(640, parent ? parent.width : 0)
    height: Math.max(480, parent ? parent.height : 0)

    CaptureFrameInfo {
        width: parent.width
    }

    TestCase {
        name: "TestCaptureFrameInfo"
        when: windowShown

        function initTestCase() {
        }

        function test_captureFrameInfo() {
            wait(100000)
        }

        function cleanupTestCase() {
        }
    }
}
