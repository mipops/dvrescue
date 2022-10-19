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

    AboutDialog {
        id: about
        version: buildVersionString
        buildDate: buildDateString
        buildQtVersion: buildQtVersionString
        runtimeQtVersion: runtimeQtVersionString
        buildFFmpegVersion: buildFFmpegVersionString
        runtimeFFmpegVersion: runtimeFFmpegVersionString
    }

    TestCase {
        name: "TestAboutDialog"
        when: windowShown
        id: testcase

        function initTestCase() {
        }

        function test_open() {
            about.open()
            wait(100000)
        }

        function cleanupTestCase() {
        }
    }
}
