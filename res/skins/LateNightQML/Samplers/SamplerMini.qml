import "../Controls" as Controls
import "../Deck" as DeckControls
import "../LateNightTheme"
import Mixxx 1.0 as Mixxx
import QtQuick
import QtQuick.Layouts

Controls.Panel {
    id: root

    property var currentTrack: player.currentTrack
    required property string group
    readonly property bool loaded: trackLoadedControl.value > 0
    property var player: Mixxx.PlayerManager.getPlayer(group)

    color: LateNightTheme.samplerPanelColor
    implicitHeight: 38
    implicitWidth: 150

    RowLayout {
        anchors.fill: parent
        anchors.margins: 1
        spacing: 1

        DeckControls.LateNightControlButton {
            Layout.fillHeight: true
            Layout.preferredWidth: 36
            activeColor: LateNightTheme.activePlayCueColor
            backgroundSource: LateNightTheme.lateNightTopRegionButton("square_big")
            displayKey: "play_latched"
            group: root.group
            iconSource: isActive ? LateNightTheme.assetSamplerPauseButton : LateNightTheme.assetSamplerPlayButton
            inactiveOpacity: 0.9
            key: "cue_gotoandplay"
            rightClickKey: "cue_default"
            stretchIcon: true
        }
        ColumnLayout {
            Layout.fillHeight: true
            Layout.fillWidth: true
            spacing: 0

            Text {
                Layout.fillWidth: true
                Layout.preferredHeight: 18
                color: LateNightTheme.samplerTitleColor
                elide: Text.ElideRight
                font.family: "Open Sans"
                font.pixelSize: 11
                text: root.currentTrack?.title ?? ""
                verticalAlignment: Text.AlignVCenter
            }
            RowLayout {
                Layout.fillHeight: true
                Layout.fillWidth: true
                spacing: 1

                Text {
                    Layout.fillWidth: true
                    color: LateNightTheme.samplerBpmColor
                    font.family: "Open Sans"
                    font.pixelSize: 10
                    text: root.loaded && visualBpmControl.value > 0 ? visualBpmControl.value.toFixed(1) : ""
                    verticalAlignment: Text.AlignVCenter
                }
                DeckControls.LateNightControlButton {
                    Layout.preferredHeight: 18
                    Layout.preferredWidth: 21
                    activeIconSuffix: "active_12"
                    backgroundSource: ""
                    group: root.group
                    iconSource: LateNightTheme.lateNightAsset("buttons", "btn__eject.svg")
                    key: "eject"
                    stretchIcon: true
                }
                SamplerOrientationButton {
                    Layout.preferredHeight: 18
                    Layout.preferredWidth: 21
                    group: root.group
                }
            }
        }
    }
    Mixxx.ControlProxy {
        id: trackLoadedControl

        group: root.group
        key: "track_loaded"
    }
    Mixxx.ControlProxy {
        id: visualBpmControl

        group: root.group
        key: "visual_bpm"
    }
    Mixxx.PlayerDropArea {
        anchors.fill: parent
        group: root.group
    }
}
