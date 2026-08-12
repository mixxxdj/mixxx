pragma ComponentBehavior: Bound

import "../Controls" as Controls
import "../Deck" as DeckControls
import "../LateNightTheme"
import "../../../qml" as Shared
import Mixxx 1.0 as Mixxx
import Mixxx.Controls 1.0 as MixxxControls
import QtQuick
import QtQuick.Layouts

Controls.Panel {
    id: root

    property var currentTrack: player.currentTrack
    required property string group
    readonly property bool loaded: trackLoadedControl.value > 0
    property var player: Mixxx.PlayerManager.getPlayer(group)
    property bool show8Hotcues: true
    property bool showFxAssignments: true

    color: LateNightTheme.samplerPanelColor
    implicitHeight: 94
    implicitWidth: 280

    RowLayout {
        anchors.fill: parent
        anchors.margins: 2
        spacing: 2

        ColumnLayout {
            Layout.fillHeight: true
            Layout.fillWidth: true
            spacing: 1

            RowLayout {
                Layout.fillWidth: true
                Layout.preferredHeight: 18
                spacing: 3

                Text {
                    Layout.fillWidth: true
                    color: LateNightTheme.samplerTitleColor
                    elide: Text.ElideRight
                    font.family: "Open Sans"
                    font.pixelSize: 11
                    text: root.currentTrack?.title ?? ""
                    verticalAlignment: Text.AlignVCenter
                }
                Rectangle {
                    Layout.fillHeight: true
                    Layout.preferredWidth: 1
                    color: LateNightTheme.deckPanelBorderDark
                }
                Text {
                    Layout.preferredWidth: 45
                    color: LateNightTheme.samplerBpmColor
                    font.family: "Open Sans"
                    font.pixelSize: 10
                    horizontalAlignment: Text.AlignHCenter
                    text: root.loaded && visualBpmControl.value > 0 ? visualBpmControl.value.toFixed(1) : ""
                    verticalAlignment: Text.AlignVCenter
                }
            }
            RowLayout {
                Layout.fillHeight: true
                Layout.fillWidth: true
                spacing: 2

                DeckControls.LateNightControlButton {
                    Layout.preferredHeight: 36
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
                Rectangle {
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    border.color: LateNightTheme.overviewBorderTopColor
                    border.width: 1
                    color: LateNightTheme.primaryOverviewBackgroundColor
                    radius: 1

                    MixxxControls.WaveformOverview {
                        anchors.fill: parent
                        anchors.margins: 1
                        channels: Mixxx.WaveformOverview.Channels.LeftChannel
                        colorHigh: LateNightTheme.white
                        colorLow: LateNightTheme.primaryWaveformSignalColor
                        colorMid: LateNightTheme.samplerColor
                        group: root.group
                        renderer: Mixxx.WaveformOverview.Renderer.Filtered
                        visible: root.loaded
                    }
                }
                GridLayout {
                    Layout.preferredHeight: 36
                    Layout.preferredWidth: 44
                    columnSpacing: 1
                    columns: 2
                    rowSpacing: 0

                    DeckControls.LateNightControlButton {
                        Layout.preferredHeight: 18
                        Layout.preferredWidth: 21
                        activeIconSuffix: "active_12"
                        backgroundSource: ""
                        group: root.group
                        iconSource: LateNightTheme.lateNightAsset("buttons", "btn__repeat.svg")
                        key: "repeat"
                        stretchIcon: true
                        toggleable: true
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
                    DeckControls.LateNightControlButton {
                        Layout.preferredHeight: 18
                        Layout.preferredWidth: 21
                        activeIconSuffix: "active_12"
                        backgroundSource: ""
                        group: root.group
                        iconSource: LateNightTheme.lateNightAsset("buttons", "btn__keylock.svg")
                        key: "keylock"
                        stretchIcon: true
                        toggleable: true
                    }
                    SamplerOrientationButton {
                        Layout.preferredHeight: 18
                        Layout.preferredWidth: 21
                        group: root.group
                    }
                }
                ColumnLayout {
                    Layout.fillHeight: true
                    Layout.preferredWidth: 38
                    spacing: 0

                    Shared.ControlKnob {
                        Layout.alignment: Qt.AlignHCenter
                        Layout.preferredHeight: 30
                        Layout.preferredWidth: 35
                        arcStart: MixxxControls.Knob.ArcStart.Minimum
                        backgroundSource: LateNightTheme.assetMicAuxGainKnobBackground
                        color: LateNightTheme.samplerGainColor
                        group: root.group
                        key: "pregain"
                        shadowSource: ""
                    }
                    DeckControls.LateNightControlButton {
                        Layout.alignment: Qt.AlignHCenter
                        Layout.preferredHeight: 22
                        Layout.preferredWidth: 26
                        activeColor: LateNightTheme.samplerColor
                        group: root.group
                        iconSource: LateNightTheme.lateNightAsset("buttons", "btn__pfl.svg")
                        key: "pfl"
                        stretchIcon: true
                        toggleable: true
                    }
                }
                Shared.VuMeter {
                    Layout.fillHeight: true
                    Layout.preferredWidth: 5
                    group: root.group
                    key: "vu_meter"
                }
            }
            RowLayout {
                Layout.fillWidth: true
                Layout.preferredHeight: 22
                spacing: 1

                Repeater {
                    model: root.show8Hotcues ? 8 : 4

                    DeckControls.LateNightControlButton {
                        required property int index

                        Layout.fillWidth: true
                        Layout.preferredHeight: 20
                        activeColor: LateNightTheme.samplerColor
                        backgroundSource: LateNightTheme.lateNightSubRegionButton("square")
                        displayKey: "hotcue_" + (index + 1) + "_status"
                        group: root.group
                        key: "hotcue_" + (index + 1) + "_activate"
                        label: String(index + 1)
                    }
                }
                Repeater {
                    model: root.showFxAssignments ? 4 : 0

                    DeckControls.LateNightControlButton {
                        required property int index

                        Layout.preferredHeight: 20
                        Layout.preferredWidth: 25
                        activeColor: LateNightTheme.samplerColor
                        group: "[EffectRack1_EffectUnit" + (index + 1) + "]"
                        key: "group_" + root.group + "_enable"
                        label: "FX" + (index + 1)
                        labelPixelSize: 8
                        toggleable: true
                    }
                }
            }
        }
        ColumnLayout {
            Layout.fillHeight: true
            Layout.preferredWidth: 30
            spacing: 1

            DeckControls.LateNightControlButton {
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredHeight: 26
                Layout.preferredWidth: 26
                activeColor: LateNightTheme.samplerColor
                activeIconSuffix: LateNightTheme.isPaleMoon ? "active" : ""
                group: root.group
                iconSource: LateNightTheme.assetSamplerSyncButton
                key: "sync_enabled"
                stretchIcon: true
                toggleable: true
            }
            Shared.ControlFader {
                Layout.alignment: Qt.AlignHCenter
                Layout.fillHeight: true
                Layout.preferredWidth: 30
                bar.color: LateNightTheme.samplerColor
                bar.margin: 5
                bar.start: 0.5
                bg: LateNightTheme.assetSamplerPitchSliderBackground
                fg: LateNightTheme.assetSamplerPitchSliderHandle
                group: root.group
                key: "rate"
                orientation: Qt.Vertical
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
