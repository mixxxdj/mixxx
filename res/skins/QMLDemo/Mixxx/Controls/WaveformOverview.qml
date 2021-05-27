import Mixxx 0.1 as Mixxx
import QtQuick 2.12
import QtQuick.Shapes 1.12

Mixxx.WaveformOverview {
    id: root

    property string group // required

    player: Mixxx.PlayerManager.getPlayer(root.group)

    Mixxx.ControlProxy {
        id: playPositionControl

        group: root.group
        key: "playposition"
    }

    MouseArea {
        id: mousearea

        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor
        hoverEnabled: true
        onPressed: {
            playPositionControl.value = mouse.x / width;
        }
        onPositionChanged: {
            if (containsPress)
                playPositionControl.value = mouse.x / width;

        }
    }

}
