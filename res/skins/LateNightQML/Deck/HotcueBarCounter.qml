import QtQuick
import Mixxx 1.0 as Mixxx
import "../LateNightTheme"

// Per-deck counter showing a "bar.beat" countdown to the next hot cue ahead of
// the play position. The engine publishes the value via the read-only
// "bars_beats_to_next_hotcue" control (bar + beat/10, counting down one beat at a
// time to 0.0 at the cue, and 0.0 when there is no beatgrid or no hot cue ahead);
// this component splits it into bar and beat.
Item {
    id: root

    required property string group

    readonly property var deckPlayer: Mixxx.PlayerManager.getPlayer(root.group)
    readonly property bool isLoaded: deckPlayer?.isLoaded ?? false

    // The engine publishes a "bar.beat" countdown encoded as bar + beat/10
    // (counts down every beat to 0.0 at the cue, e.g. 15.4, 15.3, ... 0.1, 0.0;
    // 0.0 when there is no beatgrid / no hot cue ahead).
    readonly property real barBeat: barsBeatsToNextHotcueProxy.value
    readonly property int barPart: Math.floor(root.barBeat + 0.001)
    readonly property int beatPart: Math.round((root.barBeat - root.barPart) * 10)

    implicitWidth: 54
    implicitHeight: 40

    Mixxx.ControlProxy {
        id: barsBeatsToNextHotcueProxy
        group: root.group
        key: "bars_beats_to_next_hotcue"
    }

    Rectangle {
        anchors.fill: parent
        radius: 3
        color: LateNightTheme.deckPanelColor
        border.width: 1
        border.color: LateNightTheme.deckPanelBorderDark
        opacity: 0.9
        visible: root.isLoaded
    }

    Column {
        anchors.centerIn: parent
        spacing: 0
        visible: root.isLoaded

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: root.barPart + "." + root.beatPart
            font.family: "Open Sans"
            font.pixelSize: 22
            font.bold: true
            color: LateNightTheme.deckTimeTextColor
            horizontalAlignment: Text.AlignHCenter
        }

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: "bar.beat"
            font.family: "Open Sans"
            font.pixelSize: 9
            color: LateNightTheme.textColorMuted
            horizontalAlignment: Text.AlignHCenter
        }
    }
}
