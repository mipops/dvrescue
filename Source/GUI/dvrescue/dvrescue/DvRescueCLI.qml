import QtQuick 2.0
import Qt.labs.platform 1.1
import Launcher 0.1
import FileUtils 1.0
import ConnectionUtils 1.0

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

    function queryDecks(callback) {
        var promise = new Promise((accept, reject) => {
            var launcher = launcherFactory.createObject(null, { useThread: true});
            var outputText = '';
            launcher.outputChanged.connect((outputString) => {
                outputText += outputString;
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
                console.debug('got from dvrescue: \n' + outputText);
                try {
                    var devices = JSON.parse(outputText);
                    var hasDecklink = false;
                    for(var i = 0; i < devices.length; ++i) {

                        if(devices[i].type === 'DeckLink') {
                            hasDecklink = true;
                            break;
                        }
                    }

                    if(hasDecklink) {
                        queryControls().then((result) => {
                            for(var i = 0; i < devices.length; ++i) {
                                if(devices[i].type === 'DeckLink') {
                                    console.debug('attaching controls to device: ', devices[i].name, result.outputText)
                                    devices[i].controls = result.controls;
                                }
                            }
                            accept({devices: devices, launcher: launcher, outputText: outputText});
                        }).catch((e) => {
                            accept({devices: devices, launcher: launcher, outputText: outputText});
                        })
                    } else {
                        accept({devices: devices, launcher: launcher, outputText: outputText});
                    }
                }
                catch(err) {
                    reject(err, launcher);
                }

                launcher.destroy();
            });
            launcher.processStarted.connect((pid) => {
                if(callback)
                    callback(launcher)
            });

            launcher.execute(cmd, [ "-list_devices_json" ]);
        })

        return promise;
    }

    function queryControls(callback) {
        var promise = new Promise((accept, reject) => {
            var launcher = launcherFactory.createObject(null, { useThread: true});
            var outputText = '';
            launcher.outputChanged.connect((outputString) => {
                outputText += outputString;
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

                console.debug('got from dvrescue: \n' + outputText);
                try {
                    accept({controls: JSON.parse(outputText), launcher: launcher, outputText: outputText});
                }
                catch(err) {
                    reject(err, launcher);
                }

                launcher.destroy();
            });
            launcher.processStarted.connect((pid) => {
                if(callback)
                    callback(launcher)
            });

            launcher.execute(cmd, [ "-list_controls_json" ]);
        })

        return promise;
    }

    function status(id, callback) {
        console.debug('querying status: ', id);

        var promise = new Promise((accept, reject) => {
            var launcher = launcherFactory.createObject(null, { useThread: true});
            var outputText = '';
            launcher.outputChanged.connect((outputString) => {
                outputText += outputString;
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
                console.debug('got status from dvrescue: \n' + outputText.trim());
                try {
                    accept({status: outputText.trim(), launcher: launcher, outputText: outputText});
                }
                catch(err) {
                    reject(err);
                }

                launcher.destroy();
            });
            launcher.processStarted.connect((pid) => {
                if(callback)
                    callback(launcher)
            });

            launcher.execute(cmd, ["device://" + id, "-status"]);
        })

        return promise;
    }

    function control(id, command, opts, callback) {
        console.debug('stopping: ', id);

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
                console.debug('got from dvrescue: \n' + outputText);
                try {
                    accept({launcher: launcher, outputText: outputText});
                }
                catch(err) {
                    reject(err);
                }

                launcher.destroy();
            });
            launcher.processStarted.connect((pid) => {
                if(callback)
                    callback(launcher)
            });

            launcher.execute(cmd, ['device://' + id].concat(opts).concat(['-cmd', command]));
        })

        return promise;
    }

    function capture(id, playbackBuffer, csvParser, captureCmd, opts, callback) {
        console.debug('starting capture');

        var promise = new Promise((accept, reject) => {
            var launcher = launcherFactory.createObject(null, { useThread: true });
            var result = ConnectionUtils.connectToSlotDirect(launcher, 'outputChanged(const QByteArray&)', playbackBuffer, 'write(const QByteArray&)');
            var result = ConnectionUtils.connectToSlotQueued(launcher, 'errorChanged(const QByteArray&)', csvParser, 'write(const QByteArray&)');

            launcher.errorOccurred.connect((error) => {
                try {
                    reject(error);
                }
                catch(err) {

                }

                launcher.destroy();
            });
            launcher.processFinished.connect(() => {
                try {
                    accept({'outputText' : 'capture finished'});
                }
                catch(err) {
                    reject(err);
                }

                launcher.destroy();
            });
            launcher.processStarted.connect((pid) => {
                if(callback)
                    callback(launcher)
            });

            var arguments = ['-y', 'device://' + id].concat(opts).concat(['-capture', '-cmd', captureCmd, '-m', '-', '--verbosity', '9', '--csv'])

            if(settings.endTheCaptureIftheTapeContainsNoDataFor && settings.endTheCaptureIftheTapeContainsNoDataFor !== '') {
                arguments.push('--timeout')
                arguments.push(settings.endTheCaptureIftheTapeContainsNoDataFor)
            }

            launcher.execute(cmd, arguments);
        })

        return promise;
    }

    function grab(id, file, playbackBuffer, csvParser, opts, callback) {
        console.debug('starting grab: ', file);

        var promise = new Promise((accept, reject) => {
            var launcher = launcherFactory.createObject(null, { useThread: true });

            var result = ConnectionUtils.connectToSlotDirect(launcher, 'outputChanged(const QByteArray&)', playbackBuffer, 'write(const QByteArray&)');
            var result = ConnectionUtils.connectToSlotQueued(launcher, 'errorChanged(const QByteArray&)', csvParser, 'write(const QByteArray&)');

            launcher.errorOccurred.connect((error) => {
                try {
                    reject(error);
                }
                catch(err) {

                }

                launcher.destroy();
            });
            launcher.processFinished.connect(() => {
                try {
                    accept({'outputText' : 'grab finished'});
                }
                catch(err) {
                    reject(err);
                }

                launcher.destroy();
            });
            launcher.processStarted.connect((pid) => {
                if(callback)
                    callback(launcher)
            });

            var xml = file + ".dvrescue.xml"
            var scc = file + ".scc"

            var arguments = ['-y', 'device://' + id].concat(opts).concat(['-x', xml, '-c', scc, '--cc-format', 'scc', '-m', file, '-m', '-', '--verbosity', '9', '--csv'])

            if(settings.endTheCaptureIftheTapeContainsNoDataFor && settings.endTheCaptureIftheTapeContainsNoDataFor !== '') {
                arguments.push('--timeout')
                arguments.push(settings.endTheCaptureIftheTapeContainsNoDataFor)
            }

            if(settings.saveALogOfTheCaptureProcess) {
                arguments.push('--merge-log')
                arguments.push(file + ".capture.log")
            }

            launcher.execute(cmd, arguments);
        })

        return promise;
    }

    property var pendingReports: ({})

    function makeReport(file, callback) {
        console.debug('making report: ', file);

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
                console.debug('got from dvrescue: \n' + outputText);
                try {
                    accept(file + ".dvrescue.xml");
                }
                catch(err) {
                    reject(err);
                }

                launcher.destroy();
            });
            launcher.processStarted.connect((pid) => {
                if(callback)
                    callback(launcher)
            });

            var makeReportTemplate = "-y file.dv -x file.dv.dvrescue.xml -c file.dv.dvrescue.scc";
            var arguments = makeReportTemplate.split(" ");
            for(var i = 0; i < arguments.length; ++i) {
                arguments[i] = arguments[i].replace("file.dv", file);
            }

            launcher.execute(cmd, arguments);
        }).then((reportPath) => {
            console.debug('deleting pending report: ', file);
            delete pendingReports[file]
            return reportPath
        }).catch((err) => {
            console.debug('deleting pending report: ', file);
            delete pendingReports[file]
        })

        console.debug('adding pending report: ', file);
        pendingReports[file] = promise;
        return promise;
    }

    function merge(files, outputFile, callback) {
        console.debug('starting merge: ', JSON.stringify(files, 0, 4), 'to: ', outputFile);

        var promise = new Promise((accept, reject) => {
            var launcher = launcherFactory.createObject(null, { useThread: true });
            var outputText = '';

            launcher.errorChanged.connect((errorString) => {
                console.debug('errorString: ', errorString)
            });

            launcher.outputChanged.connect((outputString) => {
                console.debug('outputString: ', outputString)
                outputText += outputString;
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
                try {
                    accept({launcher: launcher, outputText: outputText});
                }
                catch(err) {
                    reject(err);
                }

                launcher.destroy();
            });
            launcher.processStarted.connect((pid) => {
                if(callback)
                    callback(launcher)
            });

            var arguments = files.concat(['--csv', '-m', outputFile])

            launcher.execute(cmd, arguments);
        })

        return promise;
    }
}
