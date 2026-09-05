import QtQuick
import QtQuick.Layouts
import Mixxx 1.0 as Mixxx
import "../Controls" as Controls
import "../LateNightTheme"
import "../../../qml/Mixxx/Controls" as MixxxControls

Controls.Panel {
    id: root

    readonly property Mixxx.Track currentTrack: deckPlayer?.currentTrack
    readonly property Mixxx.Player deckPlayer: Mixxx.PlayerManager.getPlayer(root.group)
    readonly property color deckTextColor: secondaryDeck ? LateNightTheme.secondaryDeckTextColor : LateNightTheme.primaryDeckTextColor
    required property string group
    readonly property bool hasTrackColor: root.isLoaded && trackColor?.valid
    readonly property bool isLoaded: deckPlayer?.isLoaded ?? false
    readonly property bool showSpinnyOrCover: (showSpinniesProxy.value > 0 || showCoverArtProxy.value > 0)
            && (!showSpinnyOrCoverProxy.initialized || showSpinnyOrCoverProxy.value > 0)
    readonly property color overviewColor: secondaryDeck ? LateNightTheme.secondaryOverviewBackgroundColor : LateNightTheme.primaryOverviewBackgroundColor
    readonly property int overviewType: Math.round(overviewTypeProxy.value)
    readonly property bool secondaryDeck: root.group === "[Channel3]" || root.group === "[Channel4]"
    readonly property var trackColor: currentTrack?.color
    readonly property color waveformColor: secondaryDeck ? LateNightTheme.secondaryWaveformSignalColor : LateNightTheme.primaryWaveformSignalColor

    signal toggleFocus

    function formatPosition() {
        const seconds = Math.max(0, Math.floor(durationProxy.value * playpositionProxy.value));
        return Math.floor(seconds / 60).toString() + ":" + (seconds % 60).toString().padStart(2, "0");
    }

    color: LateNightTheme.deckPanelColor
    implicitHeight: LateNightTheme.miniDeckHeight
    implicitWidth: 620

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
    Mixxx.ControlProxy {
        id: bpmProxy

        group: root.group
        key: "bpm"
    }
    Mixxx.ControlProxy {
        id: keyProxy

        group: root.group
        key: "key"
    }
    Mixxx.ControlProxy {
        id: keyNotationProxy

        group: "[Library]"
        key: "key_notation"
    }
    Mixxx.ControlProxy {
        id: overviewTypeProxy

        group: "[Waveform]"
        key: "WaveformOverviewType"
    }
    Mixxx.ControlProxy {
        id: showSpinnyOrCoverProxy

        group: "[Skin]"
        key: "show_spinny_or_cover"
    }
    Mixxx.ControlProxy {
        id: showSpinniesProxy

        group: "[Skin]"
        key: "show_spinnies"
    }
    Mixxx.ControlProxy {
        id: showCoverArtProxy

        group: "[Skin]"
        key: "show_coverart"
    }
    RowLayout {
        anchors.fill: parent
        anchors.margins: 1
        spacing: 2

        ColumnLayout {
            Layout.fillHeight: true
            Layout.minimumWidth: 42
            Layout.preferredWidth: 42
            spacing: 0

            LateNightControlButton {
                Layout.fillWidth: true
                Layout.preferredHeight: 26
                activeBackgroundSuffix: "set"
                activeColor: LateNightTheme.activePlayCueColor
                activeIconSuffix: LateNightTheme.playCueActiveIconSuffix
                activeOpacity: 1.0
                backgroundSource: LateNightTheme.lateNightSubRegionButton("medium")
                displayKey: "cue_indicator"
                group: root.group
                iconSource: LateNightTheme.assetDeckCueButton
                inactiveOpacity: 0.82
                key: "cue_default"
                pressedActivatesFill: true
                pressedBackgroundSuffix: "active"
                pressedIconSuffix: LateNightTheme.playCueActiveIconSuffix
                rightClickKey: "cue_gotoandstop"
            }
            LateNightPlayButton {
                Layout.fillWidth: true
                Layout.preferredHeight: 26
                backgroundSource: LateNightTheme.lateNightSubRegionButton("medium")
                group: root.group
                rightClickKey: "cue_default"
            }
        }
        Rectangle {
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.minimumWidth: 250
            border.color: LateNightTheme.overviewBorderTopColor
            color: root.overviewColor

            MixxxControls.WaveformOverview {
                anchors.fill: parent
                anchors.margins: 1
                colorHigh: root.overviewType === 0 ? root.waveformColor : "#ff0000"
                colorLow: root.overviewType === 0 ? root.waveformColor : "#0000ff"
                colorMid: root.overviewType === 0 ? root.waveformColor : "#00ff00"
                group: root.group
                renderer: root.overviewType === 0 ? Mixxx.WaveformOverview.Renderer.Filtered : Mixxx.WaveformOverview.Renderer.RGB
                analyzerStatusColor: root.waveformColor
                showAnalyzerStatus: true
            }
        }
        SpinnyCoverSlot {
            Layout.preferredHeight: root.showSpinnyOrCover ? LateNightTheme.miniSpinnySize : 0
            Layout.preferredWidth: root.showSpinnyOrCover ? LateNightTheme.miniSpinnySize : 0
            group: root.group
            visible: root.showSpinnyOrCover
        }
        ColumnLayout {
            Layout.fillHeight: true
            Layout.minimumWidth: 200
            Layout.preferredWidth: 220
            spacing: 0

            RowLayout {
                Layout.fillWidth: true
                Layout.preferredHeight: 25
                spacing: 4

                Text {
                    Layout.fillWidth: true
                    color: root.deckTextColor
                    elide: Text.ElideRight
                    font.family: "Open Sans"
                    font.pixelSize: 14
                    text: root.isLoaded ? (root.currentTrack?.title || qsTr("Unknown Title")) : ""
                    verticalAlignment: Text.AlignVCenter
                }
                Text {
                    Layout.preferredWidth: 46
                    color: LateNightTheme.deckTimeTextColor
                    font.family: "Open Sans"
                    font.pixelSize: 12
                    horizontalAlignment: Text.AlignRight
                    text: root.isLoaded ? root.formatPosition() : ""
                    verticalAlignment: Text.AlignVCenter
                }
                TapHandler {
                    onDoubleTapped: root.toggleFocus()
                }
            }
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 2
                color: root.hasTrackColor ? root.trackColor : LateNightTheme.deckPanelColor
            }
            RowLayout {
                Layout.fillWidth: true
                Layout.preferredHeight: 24
                spacing: 5

                Text {
                    Layout.fillWidth: true
                    color: root.deckTextColor
                    elide: Text.ElideRight
                    font.family: "Open Sans"
                    font.pixelSize: 13
                    text: root.isLoaded ? (root.currentTrack?.artist || qsTr("Unknown Artist")) : ""
                    verticalAlignment: Text.AlignVCenter
                }
                Text {
                    Layout.preferredWidth: 42
                    color: root.deckTextColor
                    font.family: "Open Sans"
                    font.pixelSize: 12
                    horizontalAlignment: Text.AlignHCenter
                    text: root.isLoaded ? Mixxx.KeyUtils.keyToString(keyProxy.value, keyNotationProxy.value) : ""
                    verticalAlignment: Text.AlignVCenter
                }
                Text {
                    Layout.preferredWidth: 50
                    color: root.deckTextColor
                    font.family: "Open Sans"
                    font.pixelSize: 12
                    horizontalAlignment: Text.AlignRight
                    text: root.isLoaded ? bpmProxy.value.toFixed(2) : ""
                    verticalAlignment: Text.AlignVCenter
                }
            }
        }
    }
}
