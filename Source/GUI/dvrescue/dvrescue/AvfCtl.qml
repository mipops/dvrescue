import QtQuick 2.0
import Launcher 0.1

Item {
    property string avfctlCmd: "plink dave -batch /Users/test/Downloads/dvrescue-git/tools/avfctl/avfctl"

    property Component launcherFactory: Launcher {
        Component.onCompleted: {
            console.debug('launcher created...');
        }

        Component.onDestruction: {
            console.debug('launcher destroyed...');
        }
    }

    AvfCtlParser {
        id: parser
    }

    function stop(index, callback) {
        console.debug('stopping: ', index);

        var promise = new Promise((accept, reject) => {
            var launcher = launcherFactory.createObject(null);
            var outputText = '';
            launcher.errorChanged.connect((errorString) => {
                outputText += errorString;
            });
            launcher.processFinished.connect(() => {
                console.debug('got from avfctl: \n' + outputText);
                try {
                    accept({state: parser.parseStateChanged(outputText), launcher: launcher, outputText: outputText});
                }
                catch(err) {
                    reject(err);
                }

                launcher.destroy();
            });

            launcher.execute(avfctlCmd + " -cmd stop -device " + index);
            if(callback)
                callback(launcher)
        })

        return promise;
    }

    function rew(index, callback) {
        console.debug('rewinding: ', index);

        var promise = new Promise((accept, reject) => {
            var launcher = launcherFactory.createObject(null);
            var outputText = '';
            launcher.errorChanged.connect((errorString) => {
                outputText += errorString;
            });
            launcher.processFinished.connect(() => {
                console.debug('got from avfctl: \n' + outputText);
                try {
                    accept({state: parser.parseStateChanged(outputText), launcher: launcher, outputText: outputText});
                }
                catch(err) {
                    reject(err);
                }

                launcher.destroy();
            });

            launcher.execute(avfctlCmd + " -cmd rew -device " + index);
            if(callback)
                callback(launcher)
        })

        return promise;
    }

    function ff(index, callback) {
        console.debug('rewinding: ', index);

        var promise = new Promise((accept, reject) => {
            var launcher = launcherFactory.createObject(null);
            var outputText = '';
            launcher.errorChanged.connect((errorString) => {
                outputText += errorString;
            });
            launcher.processFinished.connect(() => {
                console.debug('got from avfctl: \n' + outputText);
                try {
                    accept({state: parser.parseStateChanged(outputText), launcher: launcher, outputText: outputText});
                }
                catch(err) {
                    reject(err);
                }

                launcher.destroy();
            });

            launcher.execute(avfctlCmd + " -cmd ff -device " + index);
            if(callback)
                callback(launcher)
        })

        return promise;
    }

    function play(index, callback) {
        console.debug('playing: ', index);

        var promise = new Promise((accept, reject) => {
            var launcher = launcherFactory.createObject(null);
            var outputText = '';
            launcher.errorChanged.connect((errorString) => {
                outputText += errorString;
            });
            launcher.processFinished.connect(() => {
                console.debug('got from avfctl: \n' + outputText);
                try {
                    accept({state: parser.parseStateChanged(outputText), launcher: launcher, outputText: outputText});
                }
                catch(err) {
                    reject(err);
                }

                launcher.destroy();
            });

            launcher.execute(avfctlCmd + " -cmd play -device " + index);
            if(callback)
                callback(launcher)
        })

        return promise;
    }

    function queryDecks(callback) {
        var promise = new Promise((accept, reject) => {
            var launcher = launcherFactory.createObject(null, { useThread: true});
            var outputText = '';
            launcher.errorChanged.connect((errorString) => {
                outputText += errorString;
            });
            launcher.processFinished.connect(() => {
                console.debug('got from avfctl: \n' + outputText);
                try {
                    accept({devices: parser.parseDevicesList(outputText), launcher: launcher, outputText: outputText});
                }
                catch(err) {
                    reject(err, launcher);
                }

                launcher.destroy();
            });

            launcher.execute(avfctlCmd + " -list_devices");
            if(callback)
                callback(launcher)
        })

        return promise;
    }
}
