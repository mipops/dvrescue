import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

Dialog {
    modal: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    parent: Overlay.overlay
    clip: true
    padding: 5
    topPadding: 5
    spacing: 0
    contentWidth: 320
    contentHeight: 300
    property var controlsModel;
    property var videoModesModel: ['ntsc', 'pal']
    property var videoSourcesModel: ['sdi', 'hdmi', 'optical', 'component', 'composite', 's_video']
    property var audioSourcesModel: ['embedded', 'aes_ebu', 'analog', 'analog_xlr', 'analog_rca', 'microphone']
    property var timecodesModel: ['none', 'rp188vitc', 'rp188vitc2', 'rp188ltc', 'rp188hfr', 'rp188any', 'vitc', 'vitc2', 'serial']

    property alias currentControlIndex: controlsCombobox.currentIndex
    property alias currentVideoModeIndex: videoModesCombobox.currentIndex
    property alias currentVideoSourceIndex: videoSourcesCombobox.currentIndex
    property alias currentAudioSourceIndex: audioSourcesCombobox.currentIndex
    property alias currentTimecodesIndex: timecodesCombobox.currentIndex

    standardButtons: Dialog.Ok | Dialog.Cancel

    ColumnLayout {
        id: column
        width: parent.width

        RowLayout {
            Layout.fillWidth: true

            Label {
                text: "controls"
            }

            ComboBox {
                id: controlsCombobox
                Layout.fillWidth: true

                model: controlsModel
                textRole: "name"
            }
        }

        RowLayout {
            Layout.fillWidth: true

            Label {
                text: "video mode"
            }

            ComboBox {
                id: videoModesCombobox
                Layout.fillWidth: true

                model: videoModesModel
            }
        }

        RowLayout {
            Layout.fillWidth: true

            Label {
                text: "video source"
            }

            ComboBox {
                id: videoSourcesCombobox
                Layout.fillWidth: true

                model: videoSourcesModel
            }
        }

        RowLayout {
            Layout.fillWidth: true

            Label {
                text: "audio source"
            }

            ComboBox {
                id: audioSourcesCombobox
                Layout.fillWidth: true

                model: audioSourcesModel
            }
        }

        RowLayout {
            Layout.fillWidth: true

            Label {
                text: "timecode format"
            }

            ComboBox {
                id: timecodesCombobox
                Layout.fillWidth: true

                model: timecodesModel
            }
        }
    }
}
