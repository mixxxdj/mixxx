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
    radius: 4
    rightBorderColor: LateNightTheme.mixerPanelBorderRight
    topBorderColor: LateNightTheme.mixerPanelBorderTop

    // A pad: the glyph fills the middle and the sample name sits underneath,
    // which is how VirtualDJ and Serato lay their sampler pads out.
    ColumnLayout {
        anchors.bottomMargin: LateNightTheme.isClassic ? 3 : 2
        anchors.fill: parent
        anchors.leftMargin: LateNightTheme.isClassic ? 3 : 2
        anchors.rightMargin: LateNightTheme.isClassic ? 3 : 2
        anchors.topMargin: LateNightTheme.isClassic ? 3 : 2
        spacing: 0

        DeckControls.LateNightControlButton {
            Layout.alignment: Qt.AlignHCenter
            Layout.fillHeight: true
            Layout.minimumHeight: 26
            Layout.preferredWidth: 42
            activeColor: LateNightTheme.activePlayCueColor
            backgroundSource: ""
            contentOpacity: 1
            displayKey: "play_latched"
            fillMargin: 1
            fillRadius: 3
            group: root.group
            iconName: isActive ? "pause" : "play"
            inactiveFillEnabled: false
            inactiveOpacity: 0.9
            key: "cue_gotoandplay"
            rightClickKey: "cue_default"
            solidFillEnabled: true
        }
        Text {
            Layout.fillWidth: true
            Layout.preferredHeight: 15
            color: LateNightTheme.samplerTitleColor
            elide: Text.ElideRight
            font.bold: true
            font.family: LateNightTheme.fontFamily
            font.pixelSize: 12
            horizontalAlignment: Text.AlignHCenter
            text: root.currentTrack?.title ?? ""
            verticalAlignment: Text.AlignVCenter
        }
        RowLayout {
            Layout.fillWidth: true
            Layout.preferredHeight: 14
            spacing: 1
            visible: root.loaded && root.height > 52

            Text {
                Layout.fillWidth: true
                color: LateNightTheme.samplerBpmColor
                elide: Text.ElideRight
                font.family: LateNightTheme.fontFamily
                font.pixelSize: 11
                font.weight: Font.Medium
                horizontalAlignment: Text.AlignHCenter
                text: root.loaded && visualBpmControl.value > 0 ? visualBpmControl.value.toFixed(2) : ""
                verticalAlignment: Text.AlignVCenter
            }
            DeckControls.LateNightControlButton {
                Layout.preferredHeight: 14
                Layout.preferredWidth: 17
                activeIconSuffix: "active_12"
                backgroundSource: ""
                contentOpacity: 1
                group: root.group
                iconName: "eject"
                iconSize: 11
                inactiveFillEnabled: false
                key: "eject"
                stretchIcon: true
            }
            SamplerOrientationButton {
                Layout.preferredHeight: 14
                Layout.preferredWidth: 17
                group: root.group
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
