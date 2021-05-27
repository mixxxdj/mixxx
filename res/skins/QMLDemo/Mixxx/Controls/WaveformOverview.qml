import Mixxx 0.1 as Mixxx
import Mixxx.Controls 0.1 as MixxxControls
import QtQuick 2.12
import QtQuick.Shapes 1.12

Mixxx.WaveformOverview {
    id: root

    property string group // required

    player: Mixxx.PlayerManager.getPlayer(root.group)

    Mixxx.ControlProxy {
        id: trackLoadedControl

        group: root.group
        key: "track_loaded"
        onValueChanged: markers.visible = value
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
        id: mousearea

        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor
        hoverEnabled: true
        onPressed: {
            playPositionControl.value = mouse.x / mousearea.width;
        }
        onPositionChanged: {
            if (containsPress)
                playPositionControl.value = mouse.x / width;

        }
    }

}
