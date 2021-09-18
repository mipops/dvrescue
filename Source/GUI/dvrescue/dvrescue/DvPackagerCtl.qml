import QtQuick 2.0
import Launcher 0.1
import Qt.labs.platform 1.1
import FileUtils 1.0

Item {
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
