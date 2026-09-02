pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import Mixxx 1.0 as Mixxx
import "../LateNightTheme"
import "../../../qml/Deck" as SharedDeck

Item {
    id: root

    required property string group

    readonly property Mixxx.Player deckPlayer: Mixxx.PlayerManager.getPlayer(root.group)
    readonly property Mixxx.Track currentTrack: root.deckPlayer?.currentTrack ?? null
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
        return Mixxx.DurationFormatter.format(value, Mixxx.DurationFormatter.Mode.TraditionalCoarse);
    }

    function formatTrackTime(value, mode) {
        if (!Number.isFinite(value)) {
            value = 0;
        }

        const sign = value < 0 ? "-" : "";
        return sign + Mixxx.DurationFormatter.format(Math.abs(value), mode);
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
        let nextDisplay;
        switch (Mixxx.Config.controlPositionDisplay) {
        case SharedDeck.TrackTime.Display.Elapsed:
            nextDisplay = SharedDeck.TrackTime.Display.Remaining;
            break;
        case SharedDeck.TrackTime.Display.Remaining:
            nextDisplay = SharedDeck.TrackTime.Display.Both;
            break;
        case SharedDeck.TrackTime.Display.Both:
        default:
            nextDisplay = SharedDeck.TrackTime.Display.Elapsed;
            break;
        }
        Mixxx.Config.controlPositionDisplay = nextDisplay;
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
                track: root.currentTrack
                text: root.isLoaded ? (root.currentTrack?.title || "Unknown Title") : ""
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
                track: root.currentTrack
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
                track: root.currentTrack
                text: root.isLoaded ? (root.currentTrack?.artist || "Unknown Artist") : ""
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
                track: root.currentTrack
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
