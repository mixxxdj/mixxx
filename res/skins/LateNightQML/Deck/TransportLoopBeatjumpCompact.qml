import QtQuick
import QtQuick.Layouts
import Mixxx 1.0 as Mixxx
import "../../../qml/Deck" as SharedDeck
import "../LateNightTheme"

Item {
    id: root

    required property string group
    property bool showBeatjumpControls: true
    property bool showKeyControls: true
    property bool showLoopControls: true
    property bool showVinylControls: true

    implicitHeight: LateNightTheme.deckTransportHeight

    Mixxx.ControlProxy {
        id: showFourEffectsProxy

        group: "[Skin]"
        key: "show_4effectunits"
    }
    SharedDeck.BeatSizeSpinBoxBehavior {
        id: loopSizeBehavior

        decrementKey: "beatloop_size_halve"
        group: root.group
        incrementKey: "beatloop_size_double"
        key: "beatloop_size"
    }
    SharedDeck.BeatSizeSpinBoxBehavior {
        id: beatjumpSizeBehavior

        decrementKey: "beatjump_size_halve"
        group: root.group
        incrementKey: "beatjump_size_double"
        key: "beatjump_size"
    }
    RowLayout {
        anchors.fill: parent
        spacing: 0

        RowLayout {
            Layout.fillWidth: true
            Layout.maximumWidth: 360
            Layout.preferredHeight: 26
            spacing: 0

            LateNightControlButton {
                id: playButton

                Layout.preferredHeight: 26
                Layout.preferredWidth: 68
                activeBackgroundSuffix: "active"
                activeColor: LateNightTheme.activePlayCueColor
                activeOpacity: 1.0
                backgroundSource: LateNightTheme.lateNightSubRegionButton("play")
                displayKey: "play_latched"
                group: root.group
                iconSource: playButton.isActive ? LateNightTheme.assetDeckPauseButton : LateNightTheme.assetDeckPlayMiniButton
                inactiveOpacity: 0.82
                key: "play"
                rightClickKey: "cue_set"
                toggleable: true
            }
            LateNightControlButton {
                Layout.preferredHeight: 26
                Layout.preferredWidth: 42
                activeBackgroundSuffix: "set"
                activeColor: LateNightTheme.activePlayCueColor
                activeOpacity: 1.0
                backgroundSource: LateNightTheme.lateNightSubRegionButton("medium")
                displayKey: "cue_indicator"
                group: root.group
                iconSource: LateNightTheme.assetDeckCueButton
                inactiveOpacity: 0.82
                key: "cue_default"
                pressedBackgroundSuffix: "active"
                rightClickKey: "cue_gotoandstop"
            }
            Item {
                Layout.fillWidth: true
                Layout.maximumWidth: 40
                Layout.minimumWidth: 2
                visible: root.showLoopControls
            }
            RowLayout {
                Layout.preferredHeight: 26
                spacing: 0
                visible: root.showLoopControls

                LateNightControlButton {
                    Layout.preferredHeight: 26
                    Layout.preferredWidth: 26
                    backgroundSource: LateNightTheme.lateNightSubRegionButton("square")
                    displayKey: "loop_enabled"
                    group: root.group
                    iconSource: LateNightTheme.assetDeckLoopButton
                    key: "beatloop_activate"
                    rightClickKey: "reloop_toggle"
                    toggleable: true
                }
                BeatSpinBoxPlaceholder {
                    Layout.fillWidth: true
                    Layout.maximumWidth: 72
                    Layout.minimumWidth: 46
                    Layout.preferredHeight: 26
                    interactive: true
                    preferredWidth: width
                    valueText: loopSizeBehavior.valueText

                    onStepRequested: delta => loopSizeBehavior.step(delta)
                }
            }
            Item {
                Layout.fillWidth: true
                Layout.maximumWidth: 40
                Layout.minimumWidth: 2
                visible: root.showLoopControls || root.showBeatjumpControls
            }
            BeatSpinBoxPlaceholder {
                Layout.fillWidth: true
                Layout.maximumWidth: 72
                Layout.minimumWidth: 46
                Layout.preferredHeight: 26
                interactive: true
                preferredWidth: width
                valueText: beatjumpSizeBehavior.valueText
                visible: root.showBeatjumpControls

                onStepRequested: delta => beatjumpSizeBehavior.step(delta)
            }
        }
        Item {
            Layout.preferredWidth: 2
            visible: root.showVinylControls
        }
        RowLayout {
            Layout.preferredHeight: 20
            Layout.preferredWidth: 153
            spacing: 0
            visible: root.showVinylControls

            LateNightControlButton {
                Layout.fillHeight: true
                Layout.preferredWidth: 40
                backgroundSource: LateNightTheme.lateNightTopRegionButton("medium")
                displayKey: "vinylcontrol_status"
                group: root.group
                key: "vinylcontrol_enabled"
                label: "VINYL"
                toggleable: true
            }
            LateNightCycleButton {
                Layout.fillHeight: true
                Layout.preferredWidth: 46
                backgroundSource: LateNightTheme.lateNightTopRegionButton("medium")
                group: root.group
                key: "vinylcontrol_mode"
                numStates: 3
                stateLabels: ["ABS", "REL", "CONST"]
            }
            LateNightCycleButton {
                Layout.fillHeight: true
                Layout.preferredWidth: 32
                backgroundSource: LateNightTheme.lateNightTopRegionButton("medium")
                group: root.group
                key: "vinylcontrol_cueing"
                numStates: 3
                stateLabels: ["CUE", "CUE", "HOT"]
            }
            LateNightControlButton {
                Layout.fillHeight: true
                Layout.preferredWidth: 35
                backgroundSource: LateNightTheme.lateNightTopRegionButton("medium")
                group: root.group
                key: "passthrough"
                label: "PASS"
                toggleable: true
            }
        }
        Item {
            Layout.fillWidth: true
            Layout.maximumWidth: 40
            Layout.minimumWidth: 2
            visible: root.showVinylControls
        }
        RowLayout {
            Layout.preferredHeight: 20
            spacing: 0

            Repeater {
                model: showFourEffectsProxy.value > 0 ? 4 : 2

                LateNightControlButton {
                    required property int index

                    Layout.preferredHeight: 20
                    Layout.preferredWidth: showFourEffectsProxy.value > 0 && index > 0 ? 20 : 26
                    activeColor: LateNightTheme.accentColor
                    backgroundSource: LateNightTheme.lateNightSubRegionButton("square")
                    group: "[EffectRack1_EffectUnit" + (index + 1) + "]"
                    key: "group_" + root.group + "_enable"
                    label: index === 0 ? "FX" : (index + 1).toString()
                    labelPixelSize: 9
                    toggleable: true
                }
            }
        }
        Item {
            Layout.fillWidth: true
            Layout.maximumWidth: 40
            Layout.minimumWidth: 2
            visible: root.showKeyControls
        }
        KeyControlsPlaceholder {
            Layout.preferredHeight: 20
            Layout.preferredWidth: 111
            group: root.group
            visible: root.showKeyControls
        }
        Item {
            Layout.fillWidth: true
            Layout.maximumWidth: 5
            Layout.minimumWidth: 2
            visible: root.showKeyControls
        }
    }
}
