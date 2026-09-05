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

    implicitHeight: LateNightTheme.compactDeckTransportHeight

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
    DeckControlsBackground {}

    RowLayout {
        anchors.fill: parent
        anchors.bottomMargin: LateNightTheme.optionalDeckControlsBackgroundTile.toString().length > 0 ? 2 : 0
        anchors.leftMargin: LateNightTheme.optionalDeckControlsBackgroundTile.toString().length > 0 ? 1 : 0
        anchors.rightMargin: LateNightTheme.optionalDeckControlsBackgroundTile.toString().length > 0 ? 1 : 0
        anchors.topMargin: LateNightTheme.optionalDeckControlsBackgroundTile.toString().length > 0 ? 1 : 0
        spacing: 0

        RowLayout {
            Layout.fillWidth: true
            Layout.maximumWidth: 360
            Layout.preferredHeight: 26
            spacing: 0

            LateNightPlayButton {
                Layout.preferredHeight: 26
                Layout.preferredWidth: 68
                backgroundSource: LateNightTheme.lateNightSubRegionButton("play")
                group: root.group
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
            // Legacy compact row reserves one pixel before the vinyl group;
            // the controls themselves are contiguous.
            Layout.preferredWidth: 1
            visible: root.showVinylControls
        }
        VinylControlsPlaceholder {
            Layout.preferredHeight: 20
            Layout.preferredWidth: 158
            group: root.group
            visible: root.showVinylControls
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

            LateNightFxAssignmentButtons {
                group: root.group
                showFourEffectUnits: showFourEffectsProxy.value > 0
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
