import "." as Skin
import Mixxx 1.0 as Mixxx
import QtQuick 2.12
import "Theme"

Rectangle {
    id: root

    required property string group
    property bool minimized: false
    property var deckPlayer: Mixxx.PlayerManager.getPlayer(group)
    property var currentTrack: deckPlayer.currentTrack

    color: {
        const trackColor = root.currentTrack.color;
        if (!trackColor.valid)
            return Theme.backgroundColor;

        return Qt.darker(root.currentTrack.color, 2);
    }
    implicitHeight: gainKnob.height + 10
    Drag.active: dragArea.drag.active
    Drag.dragType: Drag.Automatic
    Drag.supportedActions: Qt.CopyAction
    Drag.mimeData: {
        let data = {
            "mixxx/player": group
        };
        const trackLocationUrl = root.currentTrack.trackLocationUrl;
        if (trackLocationUrl)
            data["text/uri-list"] = trackLocationUrl;

        return data;
    }

    MouseArea {
        id: dragArea

        anchors.fill: root
        drag.target: root
    }

    Skin.SectionBackground {
        anchors.fill: parent
    }

    Skin.EmbeddedBackground {
        id: embedded

        anchors.margins: 5
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: vuMeter.left
        anchors.bottom: parent.bottom
    }

    Skin.ControlMiniKnob {
        id: gainKnob

        anchors.margins: 5
        anchors.top: parent.top
        anchors.right: parent.right
        height: 40
        width: 40
        group: root.group
        key: "pregain"
        color: Theme.samplerColor
    }

    Skin.ControlButton {
        id: playButton

        anchors.top: embedded.top
        anchors.left: embedded.left
        activeColor: Theme.samplerColor
        width: 40
        height: 40
        text: "Play"
        group: root.group
        key: "cue_gotoandplay"
        highlight: playControl.playing
    }

    Text {
        id: label

        text: root.currentTrack.title
        anchors.top: embedded.top
        anchors.left: playButton.right
        anchors.right: embedded.right
        anchors.margins: 5
        elide: Text.ElideRight
        font.family: Theme.fontFamily
        font.pixelSize: Theme.textFontPixelSize
        color: Theme.deckTextColor
    }

    Rectangle {
        id: progressContainer

        anchors.margins: 5
        anchors.left: playButton.right
        anchors.right: embedded.right
        anchors.bottom: embedded.bottom
        height: 5
        radius: height / 2
        color: "transparent"
        border.color: Theme.deckLineColor
        border.width: 1

        Rectangle {
            antialiasing: false // for performance reasons
            width: playPositionControl.value * parent.width
            height: parent.height
            radius: height / 2
            color: Theme.samplerColor

            Mixxx.ControlProxy {
                id: playPositionControl

                group: root.group
                key: "playposition"
            }
        }

        MouseArea {
            anchors.fill: progressContainer
            cursorShape: Qt.PointingHandCursor
            hoverEnabled: true
            onPressed: (mouse) => {
                playPositionControl.value = mouse.x / width;
            }
            onPositionChanged: (mouse) => {
                if (containsPress)
                    playPositionControl.value = mouse.x / width;
            }
        }
    }

    Skin.VuMeter {
        id: vuMeter

        group: root.group
        key: "vu_meter"
        width: 4
        anchors.margins: 5
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.right: gainKnob.left
    }

    Mixxx.ControlProxy {
        id: playControl

        readonly property bool playing: this.value !== 0

        function stop() {
            this.value = 0;
        }

        group: root.group
        key: "play"
    }

    Mixxx.ControlProxy {
        id: ejectControl

        group: root.group
        key: "eject"
    }

    TapHandler {
        onDoubleTapped: {
            if (playControl.playing)
                playControl.stop();
            else
                ejectControl.trigger();
        }
    }

    Mixxx.PlayerDropArea {
        anchors.fill: parent
        group: root.group
    }
}
