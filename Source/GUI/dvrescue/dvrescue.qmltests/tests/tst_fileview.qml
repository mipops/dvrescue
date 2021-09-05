import QtQuick 2.0
import QtTest 1.0
import "../../dvrescue"

Item {
    width: Math.max(1280, parent ? parent.width : 0)
    height: Math.max(1024, parent ? parent.height : 0)

    AnalyseFileViewer {
        id: fileview
        visible: false
        anchors.fill: parent

        files: [
            ":/testdata/many_attributes.dv",
            ":/testdata/many_attributes.dv.dvrescue.xml",
            ":/testdata/sample.dvrescue.xml"
        ]
    }

    TestCase {
        name: "TestFileView"
        when: windowShown

        function initTestCase() {
        }

        function test_fileList() {
            console.debug('test_fileList')
            fileview.visible = true

            wait(100000)
        }

        function cleanupTestCase() {
        }
    }
}
