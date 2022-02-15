import QtQuick 2.0
import QtTest 1.0
import "../../dvrescue"

Item {
    width: Math.max(1280, parent ? parent.width : 0)
    height: Math.max(1024, parent ? parent.height : 0)

    RecentsPopup {
        id: recentsPopup
        visible: false
    }

    TestCase {
        name: "TestPopups"
        when: windowShown

        function initTestCase() {
        }

        function test_recents() {
            recentsPopup.visible = true

            wait(100000)
        }

        function cleanupTestCase() {
        }
    }
}
