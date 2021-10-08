import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import Qt.labs.settings 1.0
import Launcher 0.1
import FileUtils 1.0
import SettingsUtils 1.0
import QwtQuick2 1.0
import QtQuick.Controls 1.4 as QQC1

Dialog {
    id: toolsDialog
    title: "Please, specify tool locations.."
    contentWidth: 480

    property alias avfctlCmd: avfctlField.text
    property alias dvrescueCmd: dvrescueField.text
    property alias ffmpegCmd: ffmpegField.text
    property alias mediaInfoCmd: mediaInfoField.text
    property alias xmlStarletCmd: xmlStarletField.text

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

    Column {
        ValidatedTextField {
            id: avfctlField
            width: 480

            placeholderText: "avfctl path..."
            selectByMouse: true
            validate: validateTool
        }

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
    }

    standardButtons: Dialog.Reset | Dialog.Cancel | Dialog.Ok
    anchors.centerIn: parent
}
