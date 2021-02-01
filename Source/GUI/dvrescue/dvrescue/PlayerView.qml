import QtQuick 2.0
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12
import QtAVPlayerUtils 1.0
import QtAV 1.7

Rectangle {
    property alias player: player
    property real fps: 0

    ColumnLayout {
        anchors.fill: parent

        MediaPlayer {
            id: player
            autoLoad: true

            onStatusChanged: {
                console.debug('status: ', status);
                if(status === MediaPlayer.Loaded)
                    fps = QtAVPlayerUtils.fps(player);
                else if(status === MediaPlayer.UnknownStatus || status === MediaPlayer.NoMedia)
                    fps =  0;
            }

            onPlaybackStateChanged: {
                console.debug('state: ', playbackState);
            }

            function waitForStateChanged(expectedState, action) {

                var promise = new Promise((resolve, reject) => {
                                                  var stateChangedHandler;
                                                  stateChangedHandler = () => {
                                                      if(player.playbackState === expectedState) {
                                                          player.playbackStateChanged.disconnect(stateChangedHandler)
                                                          resolve();
                                                      }
                                                  };

                                                  player.playbackStateChanged.connect(stateChangedHandler);
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

            function waitForStepFinished(action) {
                var promise = new Promise((resolve, reject) => {
                                                  var stepFinishedHandler;
                                                  stepFinishedHandler = (value) => {
                                                      player.stepFinished.disconnect(stepFinishedHandler)
                                                      resolve();
                                                  };

                                                  player.stepFinished.connect(stepFinishedHandler);
                                                  Qt.callLater(() => {
                                                        if(action)
                                                            action();
                                                  })
                                              });
                return promise
            }

            function seekEx(ms) {
                return waitForSeekFinished(() => { player.seek(ms) }).then(() => {
                    var displayPosition = QtAVPlayerUtils.displayPosition(player);
                    if(displayPosition > ms) {
                        player.stepBackward();
                    } else if(displayPosition < ms) {
                        player.stepForward();
                    }
                })
            }

            function playPaused(ms) {
                waitForStateChanged(MediaPlayer.PlayingState, () => { player.play() }).then(() => {
                    return waitForStateChanged(MediaPlayer.PausedState, () => { player.pause() })
                }).then(() => {
                    return waitForSeekFinished(() => { player.seek(ms) })
                }).then(() => {
                    var displayPosition = QtAVPlayerUtils.displayPosition(player);
                    if(displayPosition > ms) {
                        player.stepBackward();
                    } else if(displayPosition < ms) {
                        player.stepForward();
                    }
                })
            }

            Component.onCompleted: {
                QtAVPlayerUtils.setPauseOnEnd(player);
            }
        }

        VideoOutput2 {
            Layout.fillHeight: true
            Layout.fillWidth: true
            source: player
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
                    player.seek(player.duration * (position / (1 - size)))
                }
            }

            Connections {
                target: player
                function onPositionChanged() {
                    var relativePosition = player.position / player.duration * (1 - scroll.size)
                    if(!scroll.pressed)
                        scroll.position = relativePosition;
                }
            }
        }

        RowLayout {
            anchors.horizontalCenter: parent.horizontalCenter
            Button {
                enabled: player.status !== MediaPlayer.NoMedia
                text: "<<"
                onClicked: {
                    player.seekEx(0)
                }
            }

            Button {
                enabled: player.status !== MediaPlayer.NoMedia
                text: "<"
                onClicked: {
                    player.stepBackward()
                }
            }

            Button {
                enabled: player.status !== MediaPlayer.NoMedia
                text: player.playbackState === MediaPlayer.PausedState ? "Play" : "Pause"
                onClicked: {
                    if(player.playbackState === MediaPlayer.PausedState)
                        player.play()
                    else
                        player.pause()
                }
            }

            Button {
                enabled: player.status !== MediaPlayer.NoMedia
                text: ">"
                onClicked: {
                    player.stepForward()
                }
            }

            Button {
                enabled: player.status !== MediaPlayer.NoMedia
                text: ">>"
                onClicked: {
                    player.seek(player.duration - 1)
                }
            }
        }
    }

}
