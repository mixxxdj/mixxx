import "." as Skin
import Mixxx 1.0 as Mixxx
import QtQuick 2.12
import "Theme"

Rectangle {
    id: root

    property var currentTrack: deckPlayer.currentTrack
    property var deckPlayer: Mixxx.PlayerManager.getPlayer(group)
    required property string group
    property bool minimized: false

    Drag.active: dragArea.drag.active
    Drag.dragType: Drag.Automatic
    Drag.mimeData: {
        let data = {
            "mixxx/player": group
        };
        const trackLocationUrl = root.currentTrack?.trackLocationUrl;
        if (trackLocationUrl)
            data["text/uri-list"] = trackLocationUrl;
        return data;
    }
    Drag.supportedActions: Qt.CopyAction
    color: {
        const trackColor = root.currentTrack?.color;
        if (!trackColor.valid)
            return Theme.backgroundColor;
        return Qt.darker(root.currentTrack?.color, 2);
    }
    implicitHeight: gainKnob.height + 10

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

        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.margins: 5
        anchors.right: vuMeter.left
        anchors.top: parent.top
    }
    Skin.ControlMiniKnob {
        id: gainKnob

        anchors.margins: 5
        anchors.right: parent.right
        anchors.top: parent.top
        color: Theme.samplerColor
        group: root.group
        height: 40
        key: "pregain"
        width: 40
    }
    Skin.ControlButton {
        id: playButton

        activeColor: Theme.samplerColor
        anchors.left: embedded.left
        anchors.top: embedded.top
        group: root.group
        height: 40
        highlight: playControl.playing
        key: "cue_gotoandplay"
        text: "Play"
        width: 40
    }
    Text {
        id: label

        anchors.left: playButton.right
        anchors.margins: 5
        anchors.right: embedded.right
        anchors.top: embedded.top
        color: Theme.deckTextColor
        elide: Text.ElideRight
        font.family: Theme.fontFamily
        font.pixelSize: Theme.textFontPixelSize
        text: root.currentTrack?.title
    }
    Rectangle {
        id: progressContainer

        anchors.bottom: embedded.bottom
        anchors.left: playButton.right
        anchors.margins: 5
        anchors.right: embedded.right
        border.color: Theme.deckLineColor
        border.width: 1
        color: "transparent"
        height: 5
        radius: height / 2

        Rectangle {
            antialiasing: false // for performance reasons
            color: Theme.samplerColor
            height: parent.height
            radius: height / 2
            width: playPositionControl.value * parent.width

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

            onPositionChanged: mouse => {
                if (containsPress)
                    playPositionControl.value = mouse.x / width;
            }
            onPressed: mouse => {
                playPositionControl.value = mouse.x / width;
            }
        }
    }
    Skin.VuMeter {
        id: vuMeter

        anchors.bottom: parent.bottom
        anchors.margins: 5
        anchors.right: gainKnob.left
        anchors.top: parent.top
        group: root.group
        key: "vu_meter"
        width: 4
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
