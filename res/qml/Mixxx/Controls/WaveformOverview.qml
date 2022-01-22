import Mixxx 0.1 as Mixxx
import Mixxx.Controls 0.1 as MixxxControls
import QtQuick 2.12

Mixxx.WaveformOverview {
    id: root

    property string group // required

    player: Mixxx.PlayerManager.getPlayer(root.group)

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
                anchors.fill: parent
                group: root.group
                hotcueNumber: index + 1
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
