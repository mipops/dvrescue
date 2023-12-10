import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12
import QtAVPlayerUtils 1.0
import QtAVMediaPlayer 1.0
import Multimedia 1.0
import FileUtils 1.0

Rectangle {
    id: root
    property alias player: player
    property real fps: 0
    property alias ranges: player.ranges
    onRangesChanged: {
        player.seek(0);
    }

    property bool offsetsChang
    property real startOffset: 0
    property real endOffset: player.duration

    Binding {
        target: root
        delayed: true
        property: "ranges"
        value: Qt.vector2d(startOffset, endOffset)
    }

    onFpsChanged: {
        console.debug('fps: ', fps)
    }

    ColumnLayout {
        anchors.fill: parent

        VideoOutput {
            id: videoOutput
            Layout.fillHeight: true
            Layout.fillWidth: true
            objectName: "videoOutput"
        }

        QtAVMediaPlayer {
            id: player
            videoOutput: videoOutput
            filter: {
                var filters = [];
                if(tcButton.checked)
                    filters.push("format=rgb24,drawtext=text=%{pts\\\\:hms}:x=(w-text_w)/2:y=(h-text_h)*(4/5):box=1:boxcolor=gray@0.5:fontsize=36");
                if(fsharpButton.checked)
                    filters.push("format=rgb24,drawtext=text='frame# %{frame_num}':x=0:y=0:box=1:boxcolor=gray@0.5:fontsize=36:fontcolor=white");
                if(ccButton.checked) {
                    var filterItem = "subtitles=${PATH_TO_SCC}".replace('${PATH_TO_SCC}', FileUtils.getFilePath(player.source + ".dvrescue.scc", true))
                    if(Qt.platform.os === "windows") {
                        filterItem = filterItem.replace(/\\/g, '\\\\').replace(':', '\\:');
                        // filterItem = filterItem.replace(':', '\\:');
                    }
                    filters.push(filterItem)
                }

                return filters.length === 0 ? '' : filters.join(',');
            }

            Component.onCompleted: {
                console.debug('MediaPlayer.StoppedState: ', QtAVMediaPlayer.StoppedState);
                console.debug('MediaPlayer.PlayingState: ', QtAVMediaPlayer.PlayingState);
                console.debug('MediaPlayer.PausedState: ', QtAVMediaPlayer.PausedState);

                console.debug('MediaPlayer.NoMedia: ', QtAVMediaPlayer.NoMedia);
                console.debug('MediaPlayer.LoadedMedia: ', QtAVMediaPlayer.LoadedMedia);
                console.debug('MediaPlayer.EndOfMedia: ', QtAVMediaPlayer.EndOfMedia);
                console.debug('MediaPlayer.InvalidMedia: ', QtAVMediaPlayer.InvalidMedia);
            }

            /*
            autoLoad: true
            */

            onStatusChanged: {
                console.debug('status: ', status);
                if(status === QtAVMediaPlayer.LoadedMedia) {
                    console.debug('status: MediaPlayer.LoadedMedia');
                    fps = QtAVPlayerUtils.fps(player);
                    console.debug('status: MediaPlayer.LoadedMedia: fps = ', fps);
                }
                else if(status === QtAVMediaPlayer.NoMedia || status === QtAVMediaPlayer.InvalidMedia) {
                    console.debug('status: MediaPlayer.NoMedia || MediaPlayer.InvalidMedia');
                    fps =  0;
                    QtAVPlayerUtils.emitEmptyFrame(playerView.player);
                }
            }

            onStateChanged: {
                console.debug('state: ', state);
            }

            onStopped: {
                console.debug('stopped: ', pos, ', status: ', status, ', state: ', state);
                if(status !== QtAVMediaPlayer.EndOfMedia) {
                    QtAVPlayerUtils.emitEmptyFrame(player);
                }
            }

            function waitForStateChanged(expectedState, action) {

                var promise = new Promise((resolve, reject) => {
                                                  var stateChangedHandler;
                                                  stateChangedHandler = () => {
                                                      if(player.state === expectedState) {
                                                          player.stateChanged.disconnect(stateChangedHandler)
                                                          resolve();
                                                      }
                                                  };

                                                  player.stateChanged.connect(stateChangedHandler);
                                                  Qt.callLater(() => {
                                                        if(action)
                                                            action();
                                                  })
                                              });
                return promise

            }

            function waitForSeekFinished(action) {
                var promise = new Promise((resolve, reject) => {
                                                  var seekFinishedHandler;
                                                  seekFinishedHandler = (value) => {
                                                      player.seekFinished.disconnect(seekFinishedHandler)
                                                      resolve();
                                                  };

                                                  player.seekFinished.connect(seekFinishedHandler);
                                                  Qt.callLater(() => {
                                                        if(action)
                                                            action();
                                                  })
                                              });
                return promise
            }

            function playPaused(ms) {
                player.pause();
            }
        }

        ScrollBar {
            id: scroll
            Layout.fillWidth: true
            orientation: Qt.Horizontal
            size: 0.05
            policy: ScrollBar.AlwaysOn
            position: 0
            onPositionChanged: {
                if(pressed) {
                    console.debug('position: ', position / (1 - size))
                    var newSeekPos = player.duration * (position / (1 - size));
                    console.debug('new seek pos: ', newSeekPos)

                    player.waitForSeekFinished().then(() => { console.debug('qml seek finished: ', player.position ) });
                    player.seek(newSeekPos)
                }
            }

            Connections {
                target: player
                onPositionChanged: {
                    var relativePosition = player.position / player.duration * (1 - scroll.size)
                    console.debug('relativePosition: ', relativePosition);
                    if(!scroll.pressed)
                        scroll.position = relativePosition;
                }
            }
        }

        RowLayout {
            Layout.alignment: Qt.AlignHCenter

            Button {
                enabled: player.status !== QtAVMediaPlayer.NoMedia
                icon.source: "/icons/first-frame.svg"
                onClicked: {
                    player.seek(0)
                }
            }

            Button {
                enabled: player.status !== QtAVMediaPlayer.NoMedia
                icon.source: "/icons/previous-frame.svg"
                onClicked: {
                    player.stepBackward()
                }
            }

            Button {
                enabled: player.status !== QtAVMediaPlayer.NoMedia
                icon.source: player.state === QtAVMediaPlayer.PlayingState ? "/icons/stop.svg" : "/icons/play.svg"
                onClicked: {
                    if(player.state === QtAVMediaPlayer.PlayingState)
                        player.pause()
                    else
                        player.play()
                }
            }

            Button {
                enabled: player.status !== QtAVMediaPlayer.NoMedia
                icon.source: "/icons/next-frame.svg"
                onClicked: {
                    player.stepForward()
                }
            }

            Button {
                enabled: player.status !== QtAVMediaPlayer.NoMedia
                icon.source: "/icons/last-frame.svg"
                onClicked: {
                    player.seek(player.duration - 1)
                }
            }

            Button {
                id: tcButton
                checkable: true
                text: "TC"
            }

            Button {
                id: fsharpButton
                checkable: true
                text: "F#"
            }

            Button {
                id: ccButton
                checkable: true
                enabled: FileUtils.exists(FileUtils.getFilePath(player.source + ".dvrescue.scc"));
                text: "CC"
            }
        }
    }

}
