import QtQuick 2.14
import QtQuick.Controls 2.15

import Mixxx 1.0 as Mixxx
import Mixxx.Controls 1.0 as MixxxControls

Item {
    id: root

    required property string group
    property var deckPlayer: Mixxx.PlayerManager.getPlayer(root.group)

    enum TimerStatus {
        Forward,
        Cooldown,
        Backward
    }

    signal updated

    property string fullText
    property int index

    Rectangle {
        id: frame
        anchors.top: root.top
        anchors.bottom: root.bottom
        width: parent.width
        x: 6
        color: 'transparent'

        Text {
            id: text
            text: !trackLoadedControl.value || root.deckPlayer.title.trim().length + root.deckPlayer.artist.trim().length == 0 ? qsTr("No Track Loaded") : `${root.deckPlayer.title} - ${root.deckPlayer.artist}`.trim()
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

        property int modifier: 0

        readonly property int maxOffset: text.width - frame.width + 6

        repeat: true

        onTriggered: {
            frame.x += modifier;
            if (
                (frame.x >= 6 && root.state == "ScrollingForward")
                || (frame.x <= -maxOffset && root.state == "ScrollingBackward")) {
                root.state = "ScrollingCooldown"
            } else if (root.state == "ScrollingCooldown") {
                root.state = frame.x >= 6 ? "ScrollingBackward" : "ScrollingForward"
            }
        }
    }

    Connections {
        function onMaxOffsetChanged() {
            root.state = timer.maxOffset <= 0 ? "ScrollingIdle" : "ScrollingForward"
        }
        target: timer
    }

    Connections {
        function onXChanged(value) {
            root.updated()
        }
        target: frame
    }

    Connections {
        function onTextChanged(value) {
            root.updated()
        }
        target: text
    }

    state: "Idle"
    states: [
        State {
            name: "ScrollingIdle"
            PropertyChanges {
                target: timer
                running: false
            }
            PropertyChanges {
                target: frame
                x: 6
            }
        },
        State {
            name: "ScrollingForward"
            PropertyChanges {
                target: timer
                running: true
                modifier: 1
                interval: 15
            }
        },
        State {
            name: "ScrollingCooldown"
            PropertyChanges {
                target: timer
                running: true
                interval: 2000
            }
        },
        State {
            name: "ScrollingBackward"
            PropertyChanges {
                target: timer
                running: true
                modifier: -1
                interval: 15
            }
        }
    ]
}
