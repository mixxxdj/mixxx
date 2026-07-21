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

    function formatTrackTime(value, mode) {
        if (!Number.isFinite(value)) {
            value = 0;
        }

        const sign = value < 0 ? "-" : "";
        const absoluteValue = Math.abs(value);

        switch (mode) {
        case SharedDeck.TrackTime.Mode.Seconds:
        case SharedDeck.TrackTime.Mode.SecondsLong:
            {
                const seconds = Math.floor(absoluteValue).toString();
                const centiseconds = Math.floor((absoluteValue % 1) * 100).toString().padStart(2, "0");
                return sign + seconds.padStart(mode === SharedDeck.TrackTime.Mode.SecondsLong ? 3 : 0, "0") +
                        "." + centiseconds;
            }
        case SharedDeck.TrackTime.Mode.KiloSeconds:
            {
                const kilos = Math.floor(absoluteValue / 1000);
                const seconds = Math.floor(absoluteValue % 1000).toString().padStart(3, "0");
                const centiseconds = Math.floor((absoluteValue % 1) * 100).toString().padStart(2, "0");
                return sign + kilos.toString() + "." + seconds + " " + centiseconds;
            }
        case SharedDeck.TrackTime.Mode.HectoSeconds:
            return "???";
        default:
            break;
        }

        const totalSeconds = Math.floor(absoluteValue);
        const centiseconds = Math.floor((absoluteValue - totalSeconds) * 100);
        const hours = Math.floor(totalSeconds / 3600);
        const minutes = Math.floor((totalSeconds % 3600) / 60);
        const seconds = totalSeconds % 60;
        let text = durationProxy.value > 3600
                ? Math.floor(totalSeconds / 3600).toString().padStart(2, "0") + ":" +
                        minutes.toString().padStart(2, "0") + ":" +
                        seconds.toString().padStart(2, "0")
                : minutes.toString() + ":" + seconds.toString().padStart(2, "0");

        if (mode !== SharedDeck.TrackTime.Mode.TraditionalCoarse) {
            text += "." + centiseconds.toString().padStart(2, "0");
        }
        return sign + text;
    }

    function formatPositionTime() {
        const elapsed = durationProxy.value * playpositionProxy.value;
        const remaining = durationProxy.value * (1 - playpositionProxy.value);
        const mode = Mixxx.Config.controlTimeFormat;
        switch (Mixxx.Config.controlPositionDisplay) {
        case SharedDeck.TrackTime.Display.Remaining:
            return "-" + root.formatTrackTime(remaining, mode);
        case SharedDeck.TrackTime.Display.Both:
            return root.formatTrackTime(elapsed, mode) + "  -" + root.formatTrackTime(remaining, mode);
        case SharedDeck.TrackTime.Display.Elapsed:
        default:
            return root.formatTrackTime(elapsed, mode);
        }
    }

    function cyclePositionDisplay() {
        switch (Mixxx.Config.controlPositionDisplay) {
        case SharedDeck.TrackTime.Display.Elapsed:
            Mixxx.Config.controlPositionDisplay = SharedDeck.TrackTime.Display.Remaining;
            break;
        case SharedDeck.TrackTime.Display.Remaining:
            Mixxx.Config.controlPositionDisplay = SharedDeck.TrackTime.Display.Both;
            break;
        case SharedDeck.TrackTime.Display.Both:
        default:
            Mixxx.Config.controlPositionDisplay = SharedDeck.TrackTime.Display.Elapsed;
            break;
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
            spacing: 0

            LateNightTrackPropertyText {
                id: titleText
                Layout.fillWidth: true
                Layout.fillHeight: true
                group: root.group
                track: currentTrack
                text: root.isLoaded ? (currentTrack?.title || "Unknown Title") : ""
                displayProperty: "titleInfo"
                editProperty: "title"
                editable: true
                pixelSize: 18
                textColor: root.isLoaded ? root.loadedDeckTextColor : LateNightTheme.textColorMuted
            }

            LateNightTrackPropertyText {
                id: trackTimeDisplay
                Layout.fillHeight: true
                contextMenuEnabled: false
                displayProperty: "durationTextCentiseconds"
                editable: false
                group: root.group
                horizontalAlignment: Text.AlignRight
                horizontalPadding: 6
                pixelSize: 16
                showTrackPropertiesOnDoubleClick: false
                textColor: root.isLoaded ? LateNightTheme.deckTimeTextColor : LateNightTheme.textColorMuted
                track: currentTrack
                text: root.formatPositionTime()
                visible: root.isLoaded

                onDoubleClicked: root.cyclePositionDisplay()
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
            spacing: 0

            LateNightTrackPropertyText {
                id: artistText
                Layout.fillWidth: true
                Layout.fillHeight: true
                group: root.group
                track: currentTrack
                text: root.isLoaded ? (currentTrack?.artist || "Unknown Artist") : ""
                displayProperty: "artist"
                editProperty: "artist"
                editable: true
                pixelSize: 18
                textColor: root.isLoaded ? root.loadedDeckTextColor : LateNightTheme.textColorMuted
            }

            LateNightTrackPropertyText {
                id: durationText
                Layout.fillHeight: true
                group: root.group
                track: currentTrack
                text: root.isLoaded ? root.formatDuration(durationProxy.value) : ""
                displayProperty: "durationTextSeconds"
                editable: false
                pixelSize: 14
                textColor: root.isLoaded ? LateNightTheme.deckTimeTextColor : LateNightTheme.textColorMuted
                horizontalAlignment: Text.AlignRight
                horizontalPadding: 6
            }
        }
    }
}
