import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import Qt.labs.settings 1.0
import Launcher 0.1
import FileUtils 1.0
import QwtQuick2 1.0
import QtQuick.Controls 1.4 as QQC1

ApplicationWindow {
    id: root
    width: 1920
    height: 1280
    visible: true
    title: qsTr("DVRescue")
    color: "#2e3436"

    DvRescueLogo {
        id: dvRescueLogo
        anchors.left: parent.left
        anchors.leftMargin: 10
        anchors.top: parent.top
        anchors.topMargin: 10
    }

    Column {
        id: navigationColumn
        anchors.left: parent.left
        anchors.leftMargin: 10
        anchors.top: dvRescueLogo.bottom
        anchors.topMargin: 10
        spacing: 10

        ButtonGroup {
            id: navigationButtons
            buttons: [transferButton, analysisButton, packageButton]
        }

        NavButton {
            id: transferButton
            // text: qsTr("Transfer")
            checkable: true;
            property int index: 0
            icon.source: "icons/menu-transfer.svg"
        }
        NavButton {
            id: analysisButton
            // text: qsTr("Analysis")
            checkable: true;
            property int index: 1
            checked: true
            icon.source: "icons/menu-analysis.svg"
        }
        NavButton {
            id: packageButton
            // text: qsTr("Package")
            checkable: true;
            property int index: 2
            icon.source: "icons/menu-package.svg"
        }
        NavButton {
            // text: qsTr("Settings")
            onClicked: {
                avfctlField.text = avfctl.avfctlCmd
                avfctlField.forceActiveFocus();

                dvrescueField.text = dvrescue.dvrescueCmd
                avfctlDialog.open();
            }
            icon.source: "icons/menu-settings.svg"
        }
        NavButton {
            // text: qsTr("Help")
            icon.source: "icons/menu-help.svg"
        }
        NavButton {
            // text: qsTr("Debug")
            onClicked: {
                debugView.visible = !debugView.visible
            }
            icon.source: "icons/menu-debug.svg"
        }
    }

    StackLayout {
        anchors.left: navigationColumn.right
        anchors.leftMargin: 10
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom

        currentIndex: navigationButtons.checkedButton.index

        TransferPage {
            id: mainUI

            property var urlToPath: function(url) {
                return FileUtils.getFilePath(url);
            }

            rewindMouseArea.onClicked: {
                avfctl.rew(0, (launcher) => {
                   commandsLogs.logCommand(launcher);
                }).then((result) => {
                   commandsLogs.logResult(result.outputText);
                   return result;
                });
            }

            stopMouseArea.onClicked: {
                avfctl.stop(0, (launcher) => {
                   commandsLogs.logCommand(launcher);
                }).then((result) => {
                   commandsLogs.logResult(result.outputText);
                   return result;
                });
            }

            playMouseArea.onClicked: {
                avfctl.play(0, (launcher) => {
                   commandsLogs.logCommand(launcher);
                }).then((result) => {
                   commandsLogs.logResult(result.outputText);
                   return result;
                });
            }

            fastForwardMouseArea.onClicked: {
                avfctl.ff(0,  (launcher) => {
                    commandsLogs.logCommand(launcher);
                }).then((result) => {
                    commandsLogs.logResult(result.outputText);
                    return result;
                });
            }

            grabMouseArea.onClicked: {
                specifyPathDialog.callback = (fileUrl) => {
                    var filePath = urlToPath(fileUrl);

                    avfctl.grab(0, filePath, (launcher) => {
                       console.debug('logging grab command')
                       commandsLogs.logCommand(launcher);
                    }).then((result) => {
                       commandsLogs.logResult(result.outputText);
                       return result;
                    }).catch((e) => {
                       commandsLogs.logResult(e);
                    });
                }

                specifyPathDialog.open();
            }

            deviceNameTextField.text: devicesModel.count === 0 ? '' : devicesModel.get(0).name + " (" + devicesModel.get(0).type + ")"
        }

        AnalysePage {
            id: analysePage
        }

        Rectangle {
            color: 'blue'
        }
    }

    AvfCtl {
        id: avfctl
    }

    DvRescueCLI {
        id: dvrescue
    }

    Settings {
        id: settings;
        property alias avfctlCmd: avfctl.avfctlCmd
        property alias dvrescueCmd: dvrescue.dvrescueCmd
    }

    Dialog {
        id: avfctlDialog
        title: "Please, specify tool locations.."
        contentWidth: 480

        Column {
            TextField {
                id: avfctlField
                width: 480

                placeholderText: "avfctl tool path..."
                selectByMouse: true
            }

            TextField {
                id: dvrescueField
                width: 480

                placeholderText: "dvrescue tool path..."
                selectByMouse: true
            }
        }

        standardButtons: Dialog.Cancel | Dialog.Ok
        onAccepted: {
            avfctl.avfctlCmd = avfctlField.text
            dvrescue.dvrescueCmd = dvrescueField.text
        }
        anchors.centerIn: parent
    }

    SelectPathDialog {
        id: selectPathDialog
    }

    SpecifyPathDialog {
        id: specifyPathDialog
    }

    DevicesModel {
        id: devicesModel
    }

    DevicesModelUpdater {
        model: devicesModel
        getDevices: function() {
            return avfctl.queryDecks((launcher) => {
                statusLogs.logCommand(launcher);
            }).then((result) => {
                statusLogs.logResult(result.outputText);
                return result;
            });
        }
    }

    Window {
        id: debugView
        visible: false
        width: root.width / 2
        height: root.height

        Component.onCompleted: {
            x = root.width
            y = root.y
        }

        Rectangle {
            width: parent.width / 2
            color: "#ccffffff"

            TabBar {
                id: tabBar
                width: parent.width
                currentIndex: 0

                TabButton {
                    text: "command logs"
                }
                TabButton {
                    text: "status logs"
                }
            }

            StackLayout {
                anchors.top: tabBar.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                currentIndex: tabBar.currentIndex

                ScrollView {
                    TextArea {
                        id: commandsLogs
                        selectByMouse: true

                        function logCommand(launcher) {
                            console.debug('logging command: ', launcher.program() + ' ' + launcher.arguments().join(' '))

                            append(launcher.program() + ' ' + launcher.arguments().join(' '));
                            append('\n');
                        }

                        function logResult(result) {
                            append(result);
                            append('\n\n');
                        }

                        Component.onCompleted: {
                            append("beginning commands log");
                        }
                    }
                }

                ScrollView {
                    TextArea {
                        id: statusLogs
                        selectByMouse: true

                        function logCommand(launcher) {
                            if(text.length > 50000)
                                clear();

                            append(launcher.program() + ' ' + launcher.arguments().join(' '));
                            append('\n');
                        }

                        function logResult(result) {
                            append(result);
                            append('\n\n');
                        }

                        Component.onCompleted: {
                            append("beginning status log");
                        }
                    }
                }
            }
        }
    }
}
