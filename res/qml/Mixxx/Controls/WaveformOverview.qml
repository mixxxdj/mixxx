import Mixxx 1.0 as Mixxx
import Mixxx.Controls 1.0 as MixxxControls
import QtQuick 2.12

Mixxx.WaveformOverview {
    id: root

    property color cueMarkerColor: "red"
    required property string group
    property color introOutroMarkerColor: "blue"
    property color loopMarkerColor: "green"
    property string playPositionMarkerColor: "white"
    readonly property var player: Mixxx.PlayerManager.getPlayer(root.group)

    track: player?.currentTrack

    Mixxx.ControlProxy {
        id: trackLoadedControl

        group: root.group
        key: "track_loaded"

        onValueChanged: value => {
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

        MixxxControls.WaveformOverviewMarkerLayer {
            anchors.fill: parent
            cueColor: root.cueMarkerColor
            cueText: "C"
            group: root.group
            introOutroColor: root.introOutroMarkerColor
            introStartText: "IN"
            labelColor: "white"
            loopColor: root.loopMarkerColor
            loopStartText: "LOOP"
            outroStartText: "OUT"
            showHotcueLabels: false
            showIntroOutroLabels: false
            showLoopLabel: false
        }
        MixxxControls.WaveformOverviewMarker {
            id: playPositionMarker

            anchors.fill: parent
            color: root.playPositionMarkerColor
            group: root.group
            key: "playposition"
        }
    }
    MouseArea {
        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor
        hoverEnabled: true

        onPositionChanged: mouse => {
            if (this.containsPress)
                playPositionControl.value = mouse.x / this.width;
        }
        onPressed: mouse => {
            playPositionControl.value = mouse.x / this.width;
        }
    }
}
