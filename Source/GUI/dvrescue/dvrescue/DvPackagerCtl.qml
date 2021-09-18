import QtQuick 2.0
import Launcher 0.1
import Qt.labs.platform 1.1
import FileUtils 1.0

Item {
    property string dvrescueCmd
    onDvrescueCmdChanged: {
        if(Qt.platform.os === "windows") {
            var cygpath = FileUtils.getFilePath(StandardPaths.findExecutable("cygpath.exe"));
            var filePath = FileUtils.getFilePath(FileUtils.find("dvrescue.sh"));
            var getCygwinPathPromise = getCygwinPath(cygpath, filePath).then((r) => {
                                                                                   effectiveDvrescueCmd = r.outputText
                                                                             });
        } else {
            effectiveDvrescueCmd = dvrescueCmd
        }
    }

    property string xmlStarletCmd
    onXmlStarletCmdChanged: {
        if(Qt.platform.os === "windows") {
            var cygpath = FileUtils.getFilePath(StandardPaths.findExecutable("cygpath.exe"));
            var filePath = FileUtils.getFilePath(FileUtils.find("xml.sh"));
            var getCygwinPathPromise = getCygwinPath(cygpath, filePath).then((r) => {
                                                                                   effectiveXmlStarletCmd = r.outputText
                                                                             });
        } else {
            effectiveXmlStarletCmd = xmlStarletCmd
        }
    }

    property string mediaInfoCmd
    onMediaInfoCmdChanged: {
        if(Qt.platform.os === "windows") {
            var cygpath = FileUtils.getFilePath(StandardPaths.findExecutable("cygpath.exe"));
            var filePath = FileUtils.getFilePath(FileUtils.find("mediainfo.sh"));
            var getCygwinPathPromise = getCygwinPath(cygpath, filePath).then((r) => {
                                                                                   effectiveMediaInfoCmd = r.outputText
                                                                             });
        } else {
            effectiveMediaInfoCmd = mediaInfoCmd
        }
    }

    property string ffmpegCmd
    onFfmpegCmdChanged: {
        if(Qt.platform.os === "windows") {
            var cygpath = FileUtils.getFilePath(StandardPaths.findExecutable("cygpath.exe"));
            var filePath = FileUtils.getFilePath(FileUtils.find("ffmpeg.sh"));
            var getCygwinPathPromise = getCygwinPath(cygpath, filePath).then((r) => {
                                                                                   effectiveFfmpegCmd = r.outputText
                                                                             });
        } else {
            effectiveFfmpegCmd = ffmpegCmd
        }
    }

    property string effectiveDvrescueCmd
    onEffectiveDvrescueCmdChanged: {
        console.debug('effectiveDvrescueCmd: ', effectiveDvrescueCmd)
    }

    property string effectiveXmlStarletCmd
    onEffectiveXmlStarletCmdChanged: {
        console.debug('effectiveXmlStarletCmd: ', effectiveXmlStarletCmd)
    }

    property string effectiveMediaInfoCmd
    onEffectiveMediaInfoCmdChanged: {
        console.debug('effectiveMediaInfoCmd: ', effectiveMediaInfoCmd)
    }

    property string effectiveFfmpegCmd
    onEffectiveFfmpegCmdChanged: {
        console.debug('effectiveFfmpegCmd: ', effectiveFfmpegCmd)
    }

    property string dvpackagerName: "dvpackager"
    property var paths: []

    property string detectedDvPackagerCmd: FileUtils.find(dvpackagerName);
    onDetectedDvPackagerCmdChanged: {
        console.debug('detectedDvPackagerCmd: ', detectedDvPackagerCmd)
    }

    property string bashName: Qt.platform.os === "windows" ? "bash.exe" : "bash"
    property string detectedBashCmd: FileUtils.getFilePath(StandardPaths.findExecutable(bashName));
    onDetectedBashCmdChanged: {
        console.debug('detectedBashCmd: ', detectedBashCmd)
    }

    property string dvPackagerCmd: detectedDvPackagerCmd ? FileUtils.getFilePath(detectedDvPackagerCmd) : ''
    property Component launcherFactory: Launcher {
        Component.onCompleted: {
            console.debug('launcher created...');
        }

        Component.onDestruction: {
            console.debug('launcher destroyed...');
        }
    }

    function getCygwinPath(cygpath, path) {

        console.debug('getCygwinPath: ', path)
        var promise = new Promise((accept, reject) => {
            var launcher = launcherFactory.createObject(null);
            var outputText = '';
            launcher.outputChanged.connect((outputStringt) => {
                outputText += outputStringt;
            });
            launcher.processFinished.connect(() => {
                console.debug('got from cygpath: \n' + outputText);
                try {
                    console.debug('accepting...: ' + outputText);
                    accept({launcher: launcher, outputText: outputText});
                    console.debug('accepted...: ' + outputText);
                }
                catch(err) {
                    reject(err);
                }

                launcher.destroy();
            });

            console.debug('cygpath: ', cygpath);
            launcher.execute(cygpath + " " + path);
            /*
            if(callback)
                callback(launcher)
                */
        })

        return promise;
    }

    function exec(args, callback, extraArgs) {

        var promise = new Promise((accept, reject) => {
            var launcher = launcherFactory.createObject(null);
            var outputText = '';
            launcher.outputChanged.connect((outputStringt) => {
                outputText += outputStringt;
            });
            launcher.errorOccurred.connect((error) => {
                try {
                    reject(error);
                }
                catch(err) {

                }

                launcher.destroy();
            });
            launcher.processFinished.connect(() => {
                console.debug('got from dvpackager: \n' + outputText);
                try {
                    accept({launcher: launcher, outputText: outputText});
                }
                catch(err) {
                    reject(err);
                }

                launcher.destroy();
            });

            console.debug('dvPackagerCmd: ', dvPackagerCmd);
            var cmd = detectedBashCmd + ' ' + dvPackagerCmd
            if(extraArgs)
                cmd = cmd + ' ' + extraArgs

            if(paths.length !== 0)
                launcher.setPaths(paths);

            launcher.execute(cmd + " " + args);
            if(callback)
                callback(launcher)
        })

        return promise;
    }
}
