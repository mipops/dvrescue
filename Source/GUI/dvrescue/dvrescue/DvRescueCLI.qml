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

    DvRescueParser {
        id: parser
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
                    accept({devices: parser.parseDevicesList(outputText), launcher: launcher, outputText: outputText});
                }
                catch(err) {
                    reject(err, launcher);
                }

                launcher.destroy();
            });

            launcher.execute(cmd, [ "-list_devices_json" ]);
            if(callback)
                callback(launcher)
        })

        return promise;
    }

    function status(index, callback) {
        console.debug('querying status: ', index);

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

            launcher.execute(cmd, ["device://" + index, "-status"]);
            if(callback)
                callback(launcher)
        })

        return promise;
    }

    function control(index, command, callback) {
        console.debug('stopping: ', index);

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

            launcher.execute(cmd, ['device://' + index, '-cmd', command]);
            if(callback)
                callback(launcher)
        })

        return promise;
    }

    function capture(index, playbackBuffer, csvParser, captureCmd, callback) {
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

            var arguments = ['-y', 'device://' + index, '-capture', '-cmd', captureCmd, '-m', '-', '--verbosity', '9', '--csv']

            if(settings.endTheCaptureIftheTapeContainsNoDataFor && settings.endTheCaptureIftheTapeContainsNoDataFor !== '') {
                arguments.push('--timeout')
                arguments.push(settings.endTheCaptureIftheTapeContainsNoDataFor)
            }

            launcher.execute(cmd, arguments);
            if(callback)
                callback(launcher)
        })

        return promise;
    }

    function grab(index, file, playbackBuffer, fileWriter, csvParser, callback) {
        console.debug('starting grab: ', file, fileWriter);

        var promise = new Promise((accept, reject) => {
            var launcher = launcherFactory.createObject(null, { useThread: true });

            var result = ConnectionUtils.connectToSlotDirect(launcher, 'outputChanged(const QByteArray&)', playbackBuffer, 'write(const QByteArray&)');
            var result = ConnectionUtils.connectToSlotQueued(launcher, 'outputChanged(const QByteArray&)', fileWriter, 'write(const QByteArray&)');

            var result = ConnectionUtils.connectToSlotQueued(launcher, 'errorChanged(const QByteArray&)', csvParser, 'write(const QByteArray&)');


            /*
            launcher.errorChanged.connect((errorString) => {
                console.debug('errorString: ', errorString)
            });
            */
            // launcher.outputChanged.connect(fileWriter.write);
            /*
            launcher.outputChanged.connect((outputString) => {
                console.debug('outputString: ', outputString)
            });
            */

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

            var xml = file + ".dvrescue.xml"
            var scc = file + ".scc"

            var arguments = ['-y', 'device://' + index, '-x', xml, '-c', scc, '--cc-format', 'scc', '-m', '-', '--verbosity', '9', '--csv']

            if(settings.endTheCaptureIftheTapeContainsNoDataFor && settings.endTheCaptureIftheTapeContainsNoDataFor !== '') {
                arguments.push('--timeout')
                arguments.push(settings.endTheCaptureIftheTapeContainsNoDataFor)
            }

            if(settings.saveALogOfTheCaptureProcess) {
                arguments.push('--merge-log')
                arguments.push(file + ".capture.log")
            }

            launcher.execute(cmd, arguments);
            if(callback)
                callback(launcher)
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

            var makeReportTemplate = "-y file.dv -x file.dv.dvrescue.xml -c file.dv.dvrescue.scc";
            var arguments = makeReportTemplate.split(" ");
            for(var i = 0; i < arguments.length; ++i) {
                arguments[i] = arguments[i].replace("file.dv", file);
            }

            launcher.execute(cmd, arguments);
            if(callback)
                callback(launcher)
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
}
