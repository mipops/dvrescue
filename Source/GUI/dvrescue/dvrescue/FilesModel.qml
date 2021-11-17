import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import Qt.labs.settings 1.0
import Launcher 0.1
import FileUtils 1.0
import SettingsUtils 1.0
import MediaInfo 1.0

ListModel {
    id: filesModel

    signal appended(var fileInfo)
    signal removed(int index, var fileInfo)

    property var mediaInfoModel: null

    function add(filePath) {
        console.debug('add: ', filePath)

        var fileInfo = report.resolveRelatedInfo(filePath)
        append(fileInfo)

        console.debug('emitting appended: ', JSON.stringify(fileInfo, 0, 4))
        appended(fileInfo)
    }

    function del(index) {
        console.debug('delete: ', index)

        var fileInfo = get(index);
        remove(index);

        console.debug('emitting removed: ', index)
        removed(index, fileInfo)
    }
}
