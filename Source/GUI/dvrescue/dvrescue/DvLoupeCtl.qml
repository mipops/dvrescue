import QtQuick 2.8
import Launcher 0.1
import Qt.labs.platform 1.1
import FileUtils 1.0
import LoggingUtils 1.0

Item {
    property string dvloupeName: "dvloupe"
    property string detectedDvLoupeCmd: FileUtils.find(dvloupeName);
    onDetectedDvLoupeCmdChanged: {
        console.debug('detectedDvLoupeCmd: ', detectedDvLoupeCmd)
    }
    property string dvLoupeCmd: detectedDvLoupeCmd ? FileUtils.getFilePath(detectedDvLoupeCmd) : ''

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

    property string effectiveDvloupeCmd
    onEffectiveDvloupeCmdChanged: {
        console.debug('dvloupe: effectiveDvloupeCmd: ', effectiveDvloupeCmd)
    }

    property string effectiveXmlStarletCmd
    onEffectiveXmlStarletCmdChanged: {
        console.debug('dvloupe: effectiveXmlStarletCmd: ', effectiveXmlStarletCmd)
    }

    property string effectiveMediaInfoCmd
    onEffectiveMediaInfoCmdChanged: {
        console.debug('dvloupe: effectiveMediaInfoCmd: ', effectiveMediaInfoCmd)
    }

    property string effectiveFfmpegCmd
    onEffectiveFfmpegCmdChanged: {
        console.debug('dvloupe: effectiveFfmpegCmd: ', effectiveFfmpegCmd)
    }

    property var paths: []

    property string bashName: Qt.platform.os === "windows" ? "bash.exe" : "bash"
    property string detectedBashCmd: FileUtils.getFilePath(StandardPaths.findExecutable(bashName));
    onDetectedBashCmdChanged: {
        console.debug('detectedBashCmd: ', detectedBashCmd)
    }

    property Component launcherFactory: Launcher {
        Component.onCompleted: {
            console.debug('launcher created...');
        }

        Component.onDestruction: {
            console.debug('launcher destroyed...');
        }
    }

    LoggingCategory {
        id: dvloupeCategory
        name: "dvrescue.dvloupe"
        Component.onCompleted: {
            console.debug(dvloupeCategory, "dvrescue.dvloupe")
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

                if(LoggingUtils.isDebugEnabled(dvloupeCategory.name)) {
                    console.debug(dvloupeCategory, "got from dvloupe's stdout: ", outputStringt)
                }
            });
            launcher.errorChanged.connect((outputStringt) => {
              outputText += outputStringt;

              if(LoggingUtils.isDebugEnabled(dvloupeCategory.name)) {
                  console.debug(dvloupeCategory, "got from dvloupe's stderr: ", outputStringt)
              }
            });

            launcher.errorOccurred.connect((error) => {
                console.debug('got error from dvplay: \n' + error);
                try {
                    reject(error);
                }
                catch(err) {

                }

                launcher.destroy();
            });
            launcher.processFinished.connect(() => {
                console.debug('got from dvloupe: \n' + outputText);
                try {
                    accept({launcher: launcher, outputText: outputText});
                }
                catch(err) {
                    reject(err);
                }

                launcher.destroy();
            });

            console.debug('dvloupeCmd: ', dvLoupeCmd);
            var cmd = detectedBashCmd + ' ' + dvLoupeCmd
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
