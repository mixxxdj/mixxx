import Mixxx 1.0 as Mixxx
import Mixxx.Controls 1.0 as MixxxControls
import QtQuick 2.12

Mixxx.WaveformOverview {
    id: root

    required property string group
    readonly property var player: Mixxx.PlayerManager.getPlayer(root.group)

    track: player.currentTrack

    Mixxx.ControlProxy {
        id: trackLoadedControl

        group: root.group
        key: "track_loaded"
        onValueChanged: (value) => {
            markers.visible = value;
        }
    }

    Mixxx.ControlProxy {
        id: playPositionControl

        group: root.group
        key: "playposition"
    }

    Item {
        id: markers

        anchors.fill: parent
        visible: trackLoadedControl.value

        Repeater {
            model: 8

            MixxxControls.WaveformOverviewHotcueMarker {
                required property int index

                anchors.fill: parent
                group: root.group // qmllint disable unqualified
                hotcueNumber: this.index + 1
            }
        }

        MixxxControls.WaveformOverviewMarker {
            id: playPositionMarker

            anchors.fill: parent
            group: root.group
            key: "playposition"
        }
    }

    MouseArea {
        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor
        hoverEnabled: true
        onPressed: (mouse) => {
            playPositionControl.value = mouse.x / this.width;
        }
        onPositionChanged: (mouse) => {
            if (this.containsPress)
                playPositionControl.value = mouse.x / this.width;
        }
    }
}
