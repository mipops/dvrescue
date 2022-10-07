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

    ToolsDialog {
        id: toolsDialog
    }

    TestCase {
        name: "TestSettingsView"
        when: windowShown
        id: testcase

        function initTestCase() {
        }

        function test_settings() {
            toolsDialog.open();

            wait(100000)
        }

        function cleanupTestCase() {
        }
    }
}
