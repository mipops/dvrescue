import QtQuick 2.0
import QtTest 1.0
import QtQuick.Window 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import "../../dvrescue"

Rectangle {
    color: 'black'
    width: Math.max(1280, parent ? parent.width : 0)
    height: Math.max(1024, parent ? parent.height : 0)

    StackLayout {
        anchors.fill: parent
        currentIndex: navigationButtons.checkedButton.index

        Rectangle {
            color: 'green'
        }
        Rectangle {
            color: 'blue'
        }
    }

    Column {
        id: navigationColumn
        anchors.left: parent.left
        anchors.leftMargin: 10
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.topMargin: 10
        spacing: 10

        ButtonGroup {
            id: navigationButtons
            buttons: [captureButton, analysisButton]
        }

        NavButton {
            id: captureButton
            // text: qsTr("Capture")
            checkable: true;
            property int index: 0
            icon.source: "qrc:/icons/menu-capture.svg"
        }
        NavButton {
            id: analysisButton
            // text: qsTr("Analysis")
            checkable: true;
            property int index: 1
            checked: true
            icon.source: "qrc:/icons/menu-analysis.svg"
        }
    }

    TestCase {
        name: "TestNavbar"
        when: windowShown

        function initTestCase() {
        }

        function test_navbar() {
            wait(100000)
        }

        function cleanupTestCase() {
        }
    }
}
