import QtQuick 2.0
import Launcher 0.1
import Qt.labs.platform 1.1
import FileUtils 1.0

Item {
    property string dvpackagerName: "dvpackager"

    property string detectedDvPackagerCmd: FileUtils.find(dvpackagerName);
    onDetectedDvPackagerCmdChanged: {
        console.debug('detectedDvPackagerCmd: ', detectedDvPackagerCmd)
    }

    property string dvPackagerCmd: detectedDvPackagerCmd ? FileUtils.getFilePath(detectedDvPackagerCmd) : ''
    property var makeCmd: function(cmd) {
        console.debug('makeCmd: ', cmd)
        return cmd;
    }

    property Component launcherFactory: Launcher {
        Component.onCompleted: {
            console.debug('launcher created...');
        }

        Component.onDestruction: {
            console.debug('launcher destroyed...');
        }
    }

    function exec(args, callback) {

        var promise = new Promise((accept, reject) => {
            var launcher = launcherFactory.createObject(null);
            var outputText = '';
            launcher.outputChanged.connect((outputStringt) => {
                outputText += outputStringt;
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
            var cmd = makeCmd(dvPackagerCmd)
            launcher.execute(cmd + " " + args);
            if(callback)
                callback(launcher)
        })

        return promise;
    }
}
