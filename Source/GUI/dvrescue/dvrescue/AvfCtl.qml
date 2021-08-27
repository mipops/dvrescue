import QtQuick 2.0
import Launcher 0.1
import Qt.labs.platform 1.1
import FileUtils 1.0

Item {
    property string cmd

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

    function status(index, callback) {
        console.debug('querying status: ', index);

        var promise = new Promise((accept, reject) => {
            var launcher = launcherFactory.createObject(null);
            var outputText = '';
            launcher.errorChanged.connect((errorString) => {
                outputText += errorString;
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
                console.debug('got from avfctl: \n' + outputText);
                try {
                    accept({status: parser.parseStatus(outputText), launcher: launcher, outputText: outputText});
                }
                catch(err) {
                    reject(err);
                }

                launcher.destroy();
            });

            launcher.execute(cmd + " -status -device " + index);
            if(callback)
                callback(launcher)
        })

        return promise;
    }

    function stop(index, callback) {
        console.debug('stopping: ', index);

        var promise = new Promise((accept, reject) => {
            var launcher = launcherFactory.createObject(null);
            var outputText = '';
            launcher.errorChanged.connect((errorString) => {
                outputText += errorString;
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
                console.debug('got from avfctl: \n' + outputText);
                try {
                    accept({state: parser.parseStateChanged(outputText), launcher: launcher, outputText: outputText});
                }
                catch(err) {
                    reject(err);
                }

                launcher.destroy();
            });

            launcher.execute(cmd + " -cmd stop -device " + index);
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
            launcher.errorOccurred.connect((error) => {
                try {
                    reject(error);
                }
                catch(err) {

                }

                launcher.destroy();
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

            launcher.execute(cmd + " -cmd rew -device " + index);
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
            launcher.errorOccurred.connect((error) => {
                try {
                    reject(error);
                }
                catch(err) {

                }

                launcher.destroy();
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

            launcher.execute(cmd + " -cmd ff -device " + index);
            if(callback)
                callback(launcher)
        })

        return promise;
    }

    function grab(index, filePath, callback) {
        console.debug('grabbing: ', index, filePath);

        var promise = new Promise((accept, reject) => {
            var launcher = launcherFactory.createObject(null, { useThread: true });
            var outputText = '';
            launcher.errorChanged.connect((errorString) => {
                outputText += errorString;
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
                console.debug('got from avfctl: \n' + outputText);
                try {
                    accept({state: parser.parseStateChanged(outputText), launcher: launcher, outputText: outputText});
                }
                catch(err) {
                    reject(err);
                }

                launcher.destroy();
            });

            var grabCommand = cmd + " -cmd capture " + filePath + " -device " + index;
            console.debug('executing grab: ', grabCommand);
            launcher.execute(grabCommand);
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
            launcher.errorOccurred.connect((error) => {
                try {
                    reject(error);
                }
                catch(err) {

                }

                launcher.destroy();
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

            launcher.execute(cmd + " -cmd play -device " + index);
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
            launcher.errorOccurred.connect((error) => {
                try {
                    reject(error);
                }
                catch(err) {

                }

                launcher.destroy();
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

            launcher.execute(cmd + " -list_devices");
            if(callback)
                callback(launcher)
        })

        return promise;
    }
}
