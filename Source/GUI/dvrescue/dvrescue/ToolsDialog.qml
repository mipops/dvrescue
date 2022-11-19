import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import Qt.labs.settings 1.0
import Launcher 0.1
import FileUtils 1.0
import SettingsUtils 1.0
import QwtQuick2 1.0

Dialog {
    id: toolsDialog
    title: "Settings"
    contentWidth: 550
    contentHeight: 420

    property alias currentIndex: tabBar.currentIndex

    property alias dvrescueCmd: dvrescueField.text
    property alias ffmpegCmd: ffmpegField.text
    property alias mediaInfoCmd: mediaInfoField.text
    property alias xmlStarletCmd: xmlStarletField.text
    property alias enableDebugView: enableDebugViewCheckBox.checked
    property alias endTheCaptureIftheTapeContainsNoDataFor: endTheCaptureIftheTapeContainsNoDataForTextField.text
    property alias saveALogOfTheCaptureProcess: saveALogOfTheCaptureProcess.checked
    property alias notSaveALogOfTheCaptureProcess: notSaveALogOfTheCaptureProcess.checked

    property alias simpleFrameTable: simpleFrameTable.checked
    property alias advancedFrameTable: advancedFrameTable.checked

    property var simpleFrameTableColumns: []
    property var selectedFrameTableColumns: []

    function isToolSpecified(tool) {
        if(tool.length === 0)
            return false;

        if(!FileUtils.exists(tool))
            return false;

        return true;
    }

    function areToolsSpecified(tools) {
        for(var i = 0; i < tools.length; ++i)
            if(!isToolSpecified(tools[i]))
                return false;

        return true;
    }

    function validateTool(tool) {
        if(tool.length === 0)
            return false;

        if(!FileUtils.exists(tool)) {
            return false;
        }

        return true;
    }

    TabBar {
        id: tabBar
        width: parent.width
        currentIndex: 0

        TabButton {
            text: "Capture"
        }
        TabButton {
            text: "Analysis"
        }
        TabButton {
            text: "Advanced"
        }
    }

    StackLayout {
        id: stack
        anchors.top: tabBar.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        currentIndex: tabBar.currentIndex

        Item {
            id: capturePage

            ColumnLayout {
                width: parent.width
                anchors.top: parent.top
                anchors.topMargin: 20
                spacing: 20

                ColumnLayout {
                    Text {
                        text: "Timeout"
                        font.bold: true
                    }

                    RowLayout {
                        Text {
                            text: "End the capture if the tape contains no data for "
                        }

                        TextField {
                            id: endTheCaptureIftheTapeContainsNoDataForTextField
                            text: ""
                            validator: IntValidator {
                                bottom: 1
                                top: 12000
                            }
                        }

                        Text {
                            text: " seconds"
                        }
                    }
                }

                ColumnLayout {
                    Text {
                        text: "Log"
                        font.bold: true
                    }

                    RowLayout {
                        Text {
                            text: "Save a Log of the Capture Process:"
                        }

                        RadioButton {
                            id: saveALogOfTheCaptureProcess
                            text: "Yes"
                        }
                        RadioButton {
                            id: notSaveALogOfTheCaptureProcess
                            text: "No"
                            checked: true
                        }
                    }
                }
            }
        }

        Item {
            id: analysysPage

            ColumnLayout {
                width: parent.width
                anchors.top: parent.top
                anchors.topMargin: 20
                spacing: 20

                ColumnLayout {
                    Text {
                        text: "Frame Table display"
                        font.bold: true
                    }

                    RowLayout {
                        RadioButton {
                            id: simpleFrameTable
                            text: "Simple"
                        }
                        RadioButton {
                            id: advancedFrameTable
                            text: "Advanced"
                            checked: true
                        }
                    }

                    Column {
                        spacing: 0
                        padding: 0
                        Repeater {
                            id: frameTableColumnsSelector
                            model: selectedFrameTableColumns
                            delegate: CheckBox {
                                height: 20
                                visible: advancedFrameTable.checked
                                text: selectedFrameTableColumns[index].name
                                checked: selectedFrameTableColumns[index].selected
                                onCheckedChanged: {
                                    selectedFrameTableColumns[index].selected = checked
                                    checked = Qt.binding(() => {
                                                             return selectedFrameTableColumns[index].selected
                                                         })
                                }
                            }
                        }

                        Repeater {
                            model: simpleFrameTableColumns
                            delegate: CheckBox {
                                height: 20
                                visible: simpleFrameTable.checked
                                text: modelData
                                checked: true
                                enabled: false
                            }
                        }
                    }
                }
            }
        }

        Item {
            id: toolsPage

            Text {
                id: label
                text: "Please specify tool locations."
                font.bold: true
                horizontalAlignment: Text.AlignHCenter
                anchors.horizontalCenter: parent.horizontalCenter

                anchors.top: parent.top
                anchors.topMargin: 20
            }

            Column {
                anchors.top: label.bottom
                anchors.topMargin: 20

                ValidatedTextField {
                    id: dvrescueField
                    width: 480

                    placeholderText: "dvrescue path..."
                    selectByMouse: true
                    validate: validateTool
                }

                ValidatedTextField {
                    id: ffmpegField
                    width: 480

                    placeholderText: "ffmpeg path..."
                    selectByMouse: true
                    validate: validateTool
                }

                ValidatedTextField {
                    id: mediaInfoField
                    width: 480

                    placeholderText: "mediainfo path..."
                    selectByMouse: true
                    validate: validateTool
                }

                ValidatedTextField {
                    id: xmlStarletField
                    width: 480

                    placeholderText: "xmlstarlet path..."
                    selectByMouse: true
                    validate: validateTool
                }

                CheckBox {
                    text: "Enable Debug View"
                    id: enableDebugViewCheckBox
                }
            }
        }
    }

    standardButtons: Dialog.Reset | Dialog.Cancel | Dialog.Ok
    anchors.centerIn: parent
}
