/*
This module is used to define the top section o the screen.
Currently this section is dedicated to display title and artist of the track loaded on the deck.
*/
import QtQuick 2.14
import QtQuick.Controls 2.15

import Mixxx 1.0 as Mixxx
import Mixxx.Controls 1.0 as MixxxControls

Item {
    id: root

    required property string group
    property var deckPlayer: Mixxx.PlayerManager.getPlayer(root.group)
    readonly property var currentTrack: deckPlayer.currentTrack
    property bool scrolling: true

    property real speed: 1.7
    property real spacing: 30

    Rectangle {
        id: frame
        anchors.top: root.top
        anchors.bottom: root.bottom
        width: parent.width
        x: 6
        color: 'transparent'

        readonly property string fulltext: !trackLoadedControl.value || root.currentTrack?.title.trim().length + root.currentTrack?.artist.trim().length == 0 ? qsTr("No Track Loaded") : `${root.currentTrack?.title} - ${root.currentTrack?.artist}`.trim()

        Text {
            id: text1
            text: frame.fulltext
            font.pixelSize: 24
            font.family: "Noto Sans"
            font.letterSpacing: -1
            color: fontColor
        }
        Text {
            id: text2
            visible: root.width < text1.implicitWidth
            anchors.left: text1.right
            anchors.leftMargin: spacing
            text: frame.fulltext
            font.pixelSize: 24
            font.family: "Noto Sans"
            font.letterSpacing: -1
            color: fontColor
        }
    }

    Mixxx.ControlProxy {
        id: trackLoadedControl

        group: root.group
        key: "track_loaded"
    }

    Timer {
        id: timer

        property int modifier: -root.speed

        repeat: true
        interval: 15
        running: root.width < text1.implicitWidth && root.scrolling

        onTriggered: {
            frame.x += modifier;
            if (frame.x <= -text1.implicitWidth - spacing) {
                frame.x = 0;
            }
        }

        onRunningChanged: {
            if (!running) {
                frame.x = 6;
            }
        }
    }
}
