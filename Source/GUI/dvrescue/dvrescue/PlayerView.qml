import QtQuick 2.0
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12
import QtAVPlayerUtils 1.0
import QtMultimedia 5.12 as QtMultimedia
import MediaPlayer 1.0

Rectangle {
    property alias player: player
    property real fps: 0
    onFpsChanged: {
        console.debug('fps: ', fps)
    }

    ColumnLayout {
        anchors.fill: parent

        QtMultimedia.VideoOutput {
            id: videoOutput
            Layout.fillHeight: true
            Layout.fillWidth: true
            objectName: "videoOutput"
        }

        MediaPlayer {
            id: player
            videoOutput: videoOutput

            Component.onCompleted: {
                console.debug('MediaPlayer.StoppedState: ', MediaPlayer.StoppedState);
                console.debug('MediaPlayer.PlayingState: ', MediaPlayer.PlayingState);
                console.debug('MediaPlayer.PausedState: ', MediaPlayer.PausedState);

                console.debug('MediaPlayer.NoMedia: ', MediaPlayer.NoMedia);
                console.debug('MediaPlayer.LoadedMedia: ', MediaPlayer.LoadedMedia);
                console.debug('MediaPlayer.EndOfMedia: ', MediaPlayer.EndOfMedia);
                console.debug('MediaPlayer.InvalidMedia: ', MediaPlayer.InvalidMedia);
            }

            /*
            autoLoad: true
            */

            onStatusChanged: {
                console.debug('status: ', status);
                if(status === MediaPlayer.LoadedMedia) {
                    console.debug('status: MediaPlayer.LoadedMedia');
                    fps = QtAVPlayerUtils.fps(player);
                    console.debug('status: MediaPlayer.LoadedMedia: fps = ', fps);
                }
                else if(status === MediaPlayer.NoMedia || status === MediaPlayer.InvalidMedia) {
                    console.debug('status: MediaPlayer.NoMedia || MediaPlayer.InvalidMedia');
                    fps =  0;
                }
            }

            onStateChanged: {
                console.debug('state: ', state);
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

                    player.waitForSeekFinished().then(() => { console.debug('seek finished') });
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
                enabled: player.status !== MediaPlayer.NoMedia
                icon.source: "icons/first-frame.svg"
                onClicked: {
                    player.seek(0)
                }
            }

            Button {
                enabled: player.status !== MediaPlayer.NoMedia
                icon.source: "icons/previous-frame.svg"
                onClicked: {
                    player.stepBackward()
                }
            }

            Button {
                enabled: player.status !== MediaPlayer.NoMedia
                icon.source: player.state === MediaPlayer.PausedState ? "icons/play.svg" : "icons/stop.svg"
                onClicked: {
                    if(player.state === MediaPlayer.PausedState)
                        player.play()
                    else
                        player.pause()
                }
            }

            Button {
                enabled: player.status !== MediaPlayer.NoMedia
                icon.source: "icons/next-frame.svg"
                onClicked: {
                    player.stepForward()
                }
            }

            Button {
                enabled: player.status !== MediaPlayer.NoMedia
                icon.source: "icons/last-frame.svg"
                onClicked: {
                    player.seek(player.duration - 1)
                }
            }
        }
    }

}
