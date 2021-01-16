import QtQuick 2.0
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12
import QtAV 1.7

Rectangle {
    property alias player: player

    ColumnLayout {
        anchors.fill: parent

        MediaPlayer {
            id: player
            onStatusChanged: {
                console.debug('status: ', status);
            }

            onPlaybackStateChanged: {
                console.debug('state: ', playbackState);
            }
        }

        VideoOutput2 {
            Layout.fillHeight: true
            Layout.fillWidth: true
            source: player
        }

        RowLayout {
            anchors.horizontalCenter: parent.horizontalCenter
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
        }
    }

}
