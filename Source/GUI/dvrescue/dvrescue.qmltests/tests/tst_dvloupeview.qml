import QtQuick 2.0
import QtTest 1.0
import QtQuick.Window 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import FileUtils 1.0
import ImageUtils 1.0
import "../../dvrescue"

Rectangle {
    id: root
    width: Math.max(1280, parent ? parent.width : 0)
    height: Math.max(760, parent ? parent.height : 0)

    DvLoupeView {
        id: dvloupeView
        width: root.width
        height: root.height
    }

    TestCase {
        name: "TestDvLoupeView"
        when: windowShown

        function initTestCase() {
            var imageData = FileUtils.readBinary(":/testdata/dvplay.data")
            var imageSource = ImageUtils.toDataUri(imageData, "png")

            var dvloupeJson = JSON.parse(FileUtils.read(":/testdata/dvloupe.json"))
            dvloupeView.imageSource = imageSource
            dvloupeView.data = dvloupeJson
            dvloupeView.open();
        }

        function test_dvloupeView() {
            wait(100000)
        }

        function cleanupTestCase() {
        }
    }
}
