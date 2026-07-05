import QtQuick
import QtQuick.Layouts
import Mixxx 1.0 as Mixxx
import "../LateNightTheme"
import "../../../qml/Deck" as SharedDeck

Item {
    id: root

    required property string group

    readonly property var deckPlayer: Mixxx.PlayerManager.getPlayer(root.group)
    readonly property var currentTrack: deckPlayer?.currentTrack
    readonly property bool isLoaded: deckPlayer?.isLoaded ?? false
    readonly property bool useSecondaryDeckText: root.group === "[Channel3]" || root.group === "[Channel4]"
    readonly property color loadedDeckTextColor: useSecondaryDeckText ? LateNightTheme.secondaryDeckTextColor : LateNightTheme.primaryDeckTextColor
    readonly property var trackColor: currentTrack?.color
    readonly property string trackColorText: trackColor ? trackColor.toString().toLowerCase() : ""
    readonly property bool hasVisibleTrackColor: isLoaded && trackColor?.valid &&
            trackColorText !== "#ffffff" && trackColorText !== "#ffffffff"

    implicitHeight: 55

    function formatDuration(value) {
        if (!Number.isFinite(value) || value <= 0) {
            return "";
        }
        const totalSeconds = Math.floor(value);
        const hours = Math.floor(totalSeconds / 3600);
        const minutes = Math.floor((totalSeconds % 3600) / 60);
        const seconds = totalSeconds % 60;
        if (hours > 0) {
            return hours.toString() + ":" +
                    minutes.toString().padStart(2, "0") + ":" +
                    seconds.toString().padStart(2, "0");
        }
        return minutes.toString() + ":" + seconds.toString().padStart(2, "0");
    }

    function formatTrackTime(value, coarse) {
        if (!Number.isFinite(value)) {
            value = 0;
        }

        const absoluteValue = Math.abs(value);
        const totalSeconds = Math.floor(absoluteValue);
        const centiseconds = Math.floor((absoluteValue - totalSeconds) * 100);
        const hours = Math.floor(totalSeconds / 3600);
        const minutes = Math.floor((totalSeconds % 3600) / 60);
        const seconds = totalSeconds % 60;
        let text = hours > 0
                ? hours.toString() + ":" + minutes.toString().padStart(2, "0") + ":" + seconds.toString().padStart(2, "0")
                : minutes.toString() + ":" + seconds.toString().padStart(2, "0");

        if (!coarse) {
            text += "." + centiseconds.toString().padStart(2, "0");
        }
        return text;
    }

    function formatPositionTime() {
        const elapsed = durationProxy.value * playpositionProxy.value;
        const remaining = durationProxy.value * (1 - playpositionProxy.value);
        const coarse = Mixxx.Config.controlTimeFormat === SharedDeck.TrackTime.Mode.TraditionalCoarse;
        switch (Mixxx.Config.controlPositionDisplay) {
        case SharedDeck.TrackTime.Display.Remaining:
            return "-" + root.formatTrackTime(remaining, coarse);
        case SharedDeck.TrackTime.Display.Both:
            return root.formatTrackTime(elapsed, coarse) + "  -" + root.formatTrackTime(remaining, coarse);
        case SharedDeck.TrackTime.Display.Elapsed:
        default:
            return root.formatTrackTime(elapsed, coarse);
        }
    }

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

        // Row 1: Title and Elapsed/Remaining Time
        RowLayout {
            Layout.fillWidth: true
            Layout.preferredHeight: 25
            spacing: 10

            Text {
                id: titleText
                Layout.fillWidth: true
                text: root.isLoaded ? (currentTrack?.title || "Unknown Title") : ""
                font.family: "Open Sans"
                font.pixelSize: 18
                font.weight: Font.Normal
                color: root.isLoaded ? root.loadedDeckTextColor : LateNightTheme.textColorMuted
                elide: Text.ElideRight
                verticalAlignment: Text.AlignVCenter
            }

            Text {
                id: trackTimeDisplay
                Layout.preferredWidth: 180
                text: root.formatPositionTime()
                font.family: "Open Sans"
                font.pixelSize: 16
                font.weight: Font.Normal
                color: root.isLoaded ? LateNightTheme.deckTimeTextColor : LateNightTheme.textColorMuted
                horizontalAlignment: Text.AlignRight
                verticalAlignment: Text.AlignVCenter
                visible: root.isLoaded
            }
        }

        // Row 2: 2px Track Color Strip
        Rectangle {
            id: trackColorStrip
            Layout.fillWidth: true
            Layout.preferredHeight: 2
            color: root.hasVisibleTrackColor ? root.trackColor : "transparent"
            visible: root.hasVisibleTrackColor
        }

        // Spacer when color strip is invisible to keep height stable
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 2
            color: LateNightTheme.deckPanelColor
            visible: !trackColorStrip.visible
        }

        // Row 3: Artist and Duration
        RowLayout {
            Layout.fillWidth: true
            Layout.preferredHeight: 20
            spacing: 10

            Text {
                id: artistText
                Layout.fillWidth: true
                text: root.isLoaded ? (currentTrack?.artist || "Unknown Artist") : ""
                font.family: "Open Sans"
                font.pixelSize: 18
                font.weight: Font.Normal
                color: root.isLoaded ? root.loadedDeckTextColor : LateNightTheme.textColorMuted
                elide: Text.ElideRight
                verticalAlignment: Text.AlignVCenter
            }

            Text {
                id: durationText
                Layout.preferredWidth: 180
                text: root.isLoaded ? root.formatDuration(durationProxy.value) : ""
                font.family: "Open Sans"
                font.pixelSize: 14
                font.weight: Font.Normal
                color: root.isLoaded ? LateNightTheme.deckTimeTextColor : LateNightTheme.textColorMuted
                horizontalAlignment: Text.AlignRight
                verticalAlignment: Text.AlignVCenter
            }
        }
    }
}
