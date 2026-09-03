import QtQuick
import QtQuick.Layouts
import Mixxx 1.0 as Mixxx
import "../LateNightTheme"

Item {
    id: root

    readonly property Mixxx.Track currentTrack: deckPlayer?.currentTrack
    readonly property Mixxx.Player deckPlayer: Mixxx.PlayerManager.getPlayer(root.group)
    readonly property color deckTextColor: secondaryDeck ? LateNightTheme.secondaryDeckTextColor : LateNightTheme.primaryDeckTextColor
    required property string group
    readonly property bool isLoaded: deckPlayer?.isLoaded ?? false
    readonly property bool secondaryDeck: root.group === "[Channel3]" || root.group === "[Channel4]"
    readonly property var trackColor: currentTrack?.color

    function formatTime(seconds) {
        const value = Math.max(0, Math.floor(seconds));
        return Math.floor(value / 60).toString() + ":" + (value % 60).toString().padStart(2, "0");
    }

    implicitHeight: 46

    Mixxx.ControlProxy {
        id: durationProxy

        group: root.group
        key: "duration"
    }
    Mixxx.ControlProxy {
        id: playpositionProxy

        group: root.group
        key: "playposition"
    }
    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        RowLayout {
            Layout.fillWidth: true
            Layout.preferredHeight: 22
            spacing: 6

            Text {
                Layout.fillWidth: true
                color: root.deckTextColor
                elide: Text.ElideRight
                font.family: "Open Sans"
                font.pixelSize: 16
                text: root.isLoaded ? (root.currentTrack?.title || qsTr("Unknown Title")) : ""
                verticalAlignment: Text.AlignVCenter
            }
            Text {
                Layout.preferredWidth: 62
                color: LateNightTheme.deckTimeTextColor
                font.family: "Open Sans"
                font.pixelSize: 13
                horizontalAlignment: Text.AlignRight
                text: root.isLoaded ? root.formatTime(durationProxy.value * playpositionProxy.value) : ""
                verticalAlignment: Text.AlignVCenter
            }
        }
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 2
            color: root.isLoaded && root.trackColor?.valid ? root.trackColor : LateNightTheme.deckPanelColor
        }
        RowLayout {
            Layout.fillWidth: true
            Layout.preferredHeight: 22
            spacing: 6

            Text {
                Layout.fillWidth: true
                color: root.deckTextColor
                elide: Text.ElideRight
                font.family: "Open Sans"
                font.pixelSize: 15
                text: root.isLoaded ? (root.currentTrack?.artist || qsTr("Unknown Artist")) : ""
                verticalAlignment: Text.AlignVCenter
            }
            Text {
                Layout.preferredWidth: 62
                color: LateNightTheme.deckTimeTextColor
                font.family: "Open Sans"
                font.pixelSize: 12
                horizontalAlignment: Text.AlignRight
                text: root.isLoaded ? root.formatTime(durationProxy.value) : ""
                verticalAlignment: Text.AlignVCenter
            }
        }
    }
}
