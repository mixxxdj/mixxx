import "../Controls" as Controls
import "../Deck" as DeckControls
import "../LateNightTheme"
import Mixxx 1.0 as Mixxx
import QtQuick
import QtQuick.Layouts

Controls.Panel {
    id: root

    property var currentTrack: player?.currentTrack ?? null
    required property string group
    readonly property bool loaded: trackLoadedControl.value > 0
    property var player: Mixxx.PlayerManager.getPlayer(group)

    bottomBorderColor: LateNightTheme.mixerPanelBorderBottom
    color: LateNightTheme.samplerPanelColor
    implicitHeight: 40
    implicitWidth: 150
    leftBorderColor: LateNightTheme.mixerPanelBorderLeft
    radius: LateNightTheme.isClassic ? 2 : 1
    rightBorderColor: LateNightTheme.mixerPanelBorderRight
    topBorderColor: LateNightTheme.mixerPanelBorderTop

    RowLayout {
        anchors.bottomMargin: 2
        anchors.fill: parent
        anchors.leftMargin: LateNightTheme.isClassic ? 2 : 1
        anchors.rightMargin: 2
        anchors.topMargin: LateNightTheme.isClassic ? 2 : 1
        spacing: 0

        Item {
            Layout.fillHeight: true
            Layout.preferredWidth: 36

            DeckControls.LateNightControlButton {
                activeColor: LateNightTheme.activePlayCueColor
                anchors.centerIn: parent
                backgroundSource: LateNightTheme.lateNightTopRegionButton("square_big")
                contentOpacity: 1
                displayKey: "play_latched"
                group: root.group
                height: 34
                iconSource: isActive ? LateNightTheme.assetSamplerPauseButton : LateNightTheme.assetSamplerPlayButton
                inactiveFillEnabled: false
                inactiveOpacity: 1.0
                key: "cue_gotoandplay"
                rightClickKey: "cue_default"
                stretchIcon: true
                width: 34
            }
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
                font.bold: true
                font.family: "Open Sans"
                font.pixelSize: 14
                text: root.currentTrack?.title ?? ""
                verticalAlignment: Text.AlignVCenter
            }
            RowLayout {
                Layout.fillHeight: true
                Layout.fillWidth: true
                spacing: 1
                visible: root.loaded

                Text {
                    Layout.preferredWidth: 45
                    color: LateNightTheme.samplerBpmColor
                    font.family: "Open Sans"
                    font.pixelSize: 12
                    font.weight: Font.Medium
                    horizontalAlignment: Text.AlignLeft
                    text: root.loaded && visualBpmControl.value > 0 ? visualBpmControl.value.toFixed(2) : ""
                    verticalAlignment: Text.AlignVCenter
                }
                Item {
                    Layout.fillWidth: true
                }
                DeckControls.LateNightControlButton {
                    Layout.preferredHeight: 18
                    Layout.preferredWidth: 21
                    activeIconSuffix: "active_12"
                    backgroundSource: ""
                    contentOpacity: 1
                    group: root.group
                    iconSource: LateNightTheme.lateNightAsset("buttons", "btn__eject.svg")
                    inactiveFillEnabled: false
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
