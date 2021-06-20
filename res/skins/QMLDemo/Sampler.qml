import "." as Skin
import Mixxx 0.1 as Mixxx
import Mixxx.Controls 0.1 as MixxxControls
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.11
import "Theme"

Rectangle {
    id: root

    property string group // required
    property bool minimized: false
    property var deckPlayer: Mixxx.PlayerManager.getPlayer(group)

    color: {
        const trackColor = root.deckPlayer.color;
        if (!trackColor.valid)
            return Theme.backgroundColor;

        return Qt.darker(root.deckPlayer.color, 2);
    }
    implicitHeight: gainKnob.height + 10
    Drag.active: dragArea.drag.active
    Drag.dragType: Drag.Automatic
    Drag.supportedActions: Qt.CopyAction
    Drag.mimeData: {
        let data = {
            "mixxx/player": group
        };
        const trackLocationUrl = deckPlayer.trackLocationUrl;
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

        text: root.deckPlayer.title
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
            onPressed: {
                playPositionControl.value = mouse.x / width;
            }
            onPositionChanged: {
                if (containsPress)
                    playPositionControl.value = mouse.x / width;

            }
        }

    }

    Skin.VuMeter {
        id: vuMeter

        group: root.group
        key: "VuMeter"
        width: 4
        anchors.margins: 5
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.right: gainKnob.left
    }

    Mixxx.ControlProxy {
        id: playControl

        readonly property bool playing: value != 0

        function stop() {
            value = 0;
        }

        group: root.group
        key: "play"
    }

    Mixxx.ControlProxy {
        id: ejectControl

        function trigger() {
            value = 1;
            value = 0;
        }

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
