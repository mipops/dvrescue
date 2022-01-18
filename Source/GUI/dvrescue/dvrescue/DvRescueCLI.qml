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

    function grab(index, file, playbackBuffer, fileWriter, callback) {
        console.debug('making report: ', file, fileWriter);

        var promise = new Promise((accept, reject) => {
            var launcher = launcherFactory.createObject(null, { useThread: true });

            var result = ConnectionUtils.connectToSlotDirect(launcher, 'outputChanged(const QByteArray&)', playbackBuffer, 'write(const QByteArray&)');
            var result = ConnectionUtils.connectToSlotQueued(launcher, 'outputChanged(const QByteArray&)', fileWriter, 'write(const QByteArray&)');

            launcher.errorChanged.connect((errorString) => {
                console.debug('errorString: ', errorString)
            });

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
                    accept();
                }
                catch(err) {
                    reject(err);
                }

                launcher.destroy();
            });

            var xml = file + ".dv.dvrescue.xml"
            var scc = file + ".scc"

            var arguments = ['device://' + index, '-x', xml, '-c', scc, '--cc-format', 'scc', '-m', '-']
            // var arguments = ['device://' + index, '-m', file]

            launcher.execute(cmd + ' ' + arguments.join(' '));
            if(callback)
                callback(launcher)
        })

        return promise;
    }

    property var pendingReports: ({})

    function makeReport(file, callback) {
        console.debug('making report: ', file);

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
                console.debug('got from dvrescue: \n' + outputText);
                try {
                    accept();
                }
                catch(err) {
                    reject(err);
                }

                launcher.destroy();
            });

            var makeReportTemplate = "file.dv -x file.dv.dvrescue.xml -s file.dv.dvrescue.vtt -c file.dv.dvrescue.scc";
            var arguments = makeReportTemplate.split(" ");
            for(var i = 0; i < arguments.length; ++i) {
                arguments[i] = arguments[i].replace("file.dv", file);
            }

            launcher.execute(cmd, arguments);
            if(callback)
                callback(launcher)
        }).then(() => {
            console.debug('deleting pending report: ', file);
            delete pendingReports[file]
        }).catch((err) => {
            console.debug('deleting pending report: ', file);
            delete pendingReports[file]
        })

        console.debug('adding pending report: ', file);
        pendingReports[file] = promise;
        return promise;
    }
}
