import QtQuick 2.0
import FileUtils 1.0

QtObject {
    property string dvRescueXmlExtension: ".dvrescue.xml"

    function resolveRelatedInfo(path) {
        var fileInfo = { originalPath: path, reportPath: '', videoPath: '' }
        var extension = FileUtils.getFileExtension(path);

        if(extension === 'xml') {
            if(path.endsWith(dvRescueXmlExtension))
            {
                fileInfo.reportPath = path

                var videoPath = path.substring(0, path.length - dvRescueXmlExtension.length);
                if(FileUtils.exists(videoPath))
                {
                    fileInfo.videoPath = videoPath
                }
            }
        } else {
            fileInfo.videoPath = path

            var dvRescueXmlPath = path + dvRescueXmlExtension
            if(FileUtils.exists(dvRescueXmlPath))
            {
                fileInfo.reportPath = dvRescueXmlPath
            }
        }

        return fileInfo;
    }
}
