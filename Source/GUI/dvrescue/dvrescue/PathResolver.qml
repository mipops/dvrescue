import QtQuick 2.0
import Qt.labs.platform 1.1
import FileUtils 1.0

Item {
    function getCommandName(name) {
        return Qt.platform.os === "windows" ? (name + ".exe") : name
    }

    function resolve(name) {
        return FileUtils.getFilePath(StandardPaths.findExecutable(getCommandName(name)))
    }
}
