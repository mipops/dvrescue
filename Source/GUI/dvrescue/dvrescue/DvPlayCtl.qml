import QtQuick 2.8
import Launcher 0.1
import Qt.labs.platform 1.1
import FileUtils 1.0
import LoggingUtils 1.0

Item {
    property alias dvplayCategory: dvplayCategory
    property string dvplayName: "dvplay"
    property string detectedDvPlayCmd: FileUtils.find(dvplayName);
    onDetectedDvPlayCmdChanged: {
        console.debug('detectedDvPlayCmd: ', detectedDvPlayCmd)
    }
    property string dvPlayCmd: detectedDvPlayCmd ? FileUtils.getFilePath(detectedDvPlayCmd) : ''

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

    property string effectiveDvplayCmd
    onEffectiveDvplayCmdChanged: {
        console.debug('effectiveDvplayCmd: ', effectiveDvplayCmd)
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
        id: dvplayCategory
        name: "dvrescue.dvplay"
        Component.onCompleted: {
            console.debug(dvplayCategory, "dvrescue.dvplay")
        }
    }

    function getCygwinPath(cygpath, path) {

        console.debug('getCygwinPath: ', path)
        var promise = new Promise((accept, reject) => {
            var launcher = launcherFactory.createObject(null);
            var outputText = '';
            launcher.outputChanged.connect((outputString) => {
                outputText += outputString;
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
            launcher.execute(cygpath, [path]);
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
            var output;
            launcher.outputChanged.connect((out) => {
                output = out;
                if(LoggingUtils.isDebugEnabled(dvplayCategory.name)) {
                   const byteToHex = [];

                   for (let n = 0; n <= 0xff; ++n)
                   {
                       const hexOctet = n.toString(16).padStart(2, "0");
                       byteToHex.push(hexOctet);
                   }

                   function hex(arrayBuffer)
                   {
                       const buff = new Uint8Array(arrayBuffer);
                       const hexOctets = []; // new Array(buff.length) is even faster (preallocates necessary array size), then use hexOctets[i] instead of .push()

                       for (let i = 0; i < buff.length; ++i)
                           hexOctets.push(byteToHex[buff[i]]);

                       return hexOctets.join("");
                   }

                   console.debug(dvplayCategory, "got from dvplay's stdout: ", hex(out))
                }
            });

            launcher.errorChanged.connect((outputString) => {
                if(LoggingUtils.isDebugEnabled(dvplayCategory.name)) {
                    console.debug(dvplayCategory, "got from dvplay's stderr: ", outputString)
                }
                outputText += outputString;
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
                console.debug('got from dvplay: \n' + outputText);
                try {
                    accept({launcher: launcher, outputText: outputText, output: output});
                }
                catch(err) {
                    reject(err);
                }

                launcher.destroy();
            });

            console.debug('dvplayCmd: ', dvPlayCmd);
            var cmd = [dvPlayCmd]
            if(extraArgs)
                cmd.push(...extraArgs)
            cmd.push(...args)

            if(paths.length !== 0)
                launcher.setPaths(paths);

            launcher.execute(detectedBashCmd, cmd);
            if(callback)
                callback(launcher)
        })

        return promise;
    }
}
