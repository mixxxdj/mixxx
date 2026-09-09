pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import Mixxx 1.0 as Mixxx
import "../LateNightTheme"

Item {
    id: root

    required property string group
    property bool show8Hotcues: true
    property bool showBeatjumpControls: true
    property bool showHotcues: true
    property bool showIntroOutroCues: true
    property bool showLoopControls: true

    clip: true
    height: LateNightTheme.deckTransportHeight

    Mixxx.ControlProxy {
        id: beatloopSizeProxy

        group: root.group
        key: "beatloop_size"
    }
    Mixxx.ControlProxy {
        id: beatjumpSizeProxy

        group: root.group
        key: "beatjump_size"
    }
    DeckControlsBackground {}
    RowLayout {
        anchors.bottomMargin: LateNightTheme.optionalDeckControlsBackgroundTile.toString().length > 0 ? 2 : 0
        anchors.fill: parent
        anchors.leftMargin: LateNightTheme.optionalDeckControlsBackgroundTile.toString().length > 0 ? 1 : 0
        anchors.rightMargin: LateNightTheme.optionalDeckControlsBackgroundTile.toString().length > 0 ? 1 : 0
        anchors.topMargin: LateNightTheme.optionalDeckControlsBackgroundTile.toString().length > 0 ? 1 : 0
        spacing: 6

        GridLayout {
            Layout.preferredHeight: 52
            Layout.preferredWidth: 68
            columnSpacing: 0
            columns: 2
            rowSpacing: 0
            rows: 2

            // Cue button: left-click = cue_default, right-click = cue_gotoandstop
            // Display from cue_indicator
            LateNightControlButton {
                Layout.preferredHeight: 26
                Layout.preferredWidth: 42
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

            // Reverse button: left-click = reverse, right-click = reverseroll
            LateNightControlButton {
                Layout.preferredHeight: 26
                Layout.preferredWidth: 26
                activeBackgroundSuffix: "active"
                activeColor: LateNightTheme.activePlayCueColor
                activeIconSuffix: LateNightTheme.playCueActiveIconSuffix
                activeOpacity: 1.0
                backgroundSource: LateNightTheme.lateNightSubRegionButton("square")
                group: root.group
                iconSource: LateNightTheme.assetDeckReverseButton
                inactiveOpacity: 0.82
                key: "reverse"
                rightClickKey: "reverseroll"
                stretchIcon: true
            }

            LateNightPlayButton {
                Layout.columnSpan: 2
                Layout.preferredHeight: 26
                Layout.preferredWidth: 68
                backgroundSource: LateNightTheme.lateNightSubRegionButton("play")
                group: root.group
            }
        }
        Item {
            Layout.preferredWidth: 4
            visible: root.showHotcues || root.showIntroOutroCues
        }

        // Hotcue controls
        GridLayout {
            Layout.preferredHeight: 52
            Layout.preferredWidth: root.show8Hotcues ? 104 : 52
            columnSpacing: 0
            columns: root.show8Hotcues ? 4 : 2
            rowSpacing: 0
            rows: 2
            visible: root.showHotcues

            LateNightHotcueButton {
                Layout.preferredWidth: 26
                Layout.preferredHeight: 26
                group: root.group
                hotcueNumber: 1
            }

            LateNightHotcueButton {
                Layout.preferredWidth: 26
                Layout.preferredHeight: 26
                group: root.group
                hotcueNumber: 2
            }

            LateNightHotcueButton {
                Layout.preferredWidth: 26
                Layout.preferredHeight: 26
                group: root.group
                hotcueNumber: 3
            }

            LateNightHotcueButton {
                Layout.preferredWidth: 26
                Layout.preferredHeight: 26
                group: root.group
                hotcueNumber: 4
            }

            LateNightHotcueButton {
                visible: root.show8Hotcues
                Layout.preferredWidth: 26
                Layout.preferredHeight: 26
                group: root.group
                hotcueNumber: 5
            }

            LateNightHotcueButton {
                visible: root.show8Hotcues
                Layout.preferredWidth: 26
                Layout.preferredHeight: 26
                group: root.group
                hotcueNumber: 6
            }

            LateNightHotcueButton {
                visible: root.show8Hotcues
                Layout.preferredWidth: 26
                Layout.preferredHeight: 26
                group: root.group
                hotcueNumber: 7
            }

            LateNightHotcueButton {
                visible: root.show8Hotcues
                Layout.preferredWidth: 26
                Layout.preferredHeight: 26
                group: root.group
                hotcueNumber: 8
            }
        }
        Item {
            Layout.preferredWidth: 4
            visible: root.showHotcues || root.showIntroOutroCues
        }

        // Intro/Outro controls
        GridLayout {
            Layout.preferredHeight: 52
            Layout.preferredWidth: 52
            columnSpacing: 0
            columns: 2
            rowSpacing: 0
            rows: 2
            visible: root.showIntroOutroCues

            LateNightSpecialCueButton {
                Layout.preferredWidth: 26
                Layout.preferredHeight: 26
                group: root.group
                cueType: "intro_start"
            }

            LateNightSpecialCueButton {
                Layout.preferredWidth: 26
                Layout.preferredHeight: 26
                group: root.group
                cueType: "intro_end"
            }

            LateNightSpecialCueButton {
                Layout.preferredWidth: 26
                Layout.preferredHeight: 26
                group: root.group
                cueType: "outro_start"
            }

            LateNightSpecialCueButton {
                Layout.preferredWidth: 26
                Layout.preferredHeight: 26
                group: root.group
                cueType: "outro_end"
            }
        }
        Item {
            Layout.preferredWidth: 2
            visible: root.showIntroOutroCues || root.showLoopControls || root.showBeatjumpControls
        }
        Item {
            Layout.fillWidth: true
            Layout.maximumWidth: 80
            visible: root.showIntroOutroCues || root.showLoopControls || root.showBeatjumpControls
        }

        // Loop controls
        GridLayout {
            Layout.preferredHeight: 52
            Layout.preferredWidth: 104
            columnSpacing: 0
            columns: 4
            rowSpacing: 0
            rows: 2
            visible: root.showLoopControls

            LateNightControlButton {
                Layout.preferredWidth: 26
                Layout.preferredHeight: 26
                backgroundSource: LateNightTheme.lateNightSubRegionButton("square")
                iconSource: LateNightTheme.assetDeckLoopButton
                group: root.group
                key: "beatloop_activate"
                rightClickKey: "beatlooproll_activate"
                displayKey: "loop_enabled"
                activeBackgroundSuffix: "set"
                pressedBackgroundSuffix: "active"
                activeIconSuffix: LateNightTheme.isPaleMoon ? "active" : ""
                pressedIconSuffix: LateNightTheme.isPaleMoon ? "active" : ""
                activeOpacity: 1.0
                inactiveOpacity: 0.82
                activeColor: LateNightTheme.activePlayCueColor
                inactiveColor: LateNightTheme.deckDimButtonInactiveColor
                pressedActivatesFill: true
            }

            LateNightBeatSpinBox {
                Layout.columnSpan: 3
                Layout.preferredHeight: 26
                group: root.group
                key: "beatloop_size"
                decrementKey: "loop_halve"
                incrementKey: "loop_double"
            }

            LateNightControlButton {
                Layout.preferredWidth: 26
                Layout.preferredHeight: 26
                backgroundSource: LateNightTheme.lateNightSubRegionButton("square")
                iconSource: LateNightTheme.assetDeckReloopButton
                group: root.group
                key: "reloop_toggle"
                rightClickKey: "reloop_andstop"
                activeBackgroundSuffix: "active"
                pressedBackgroundSuffix: "active"
                activeIconSuffix: LateNightTheme.isPaleMoon ? "active" : ""
                pressedIconSuffix: LateNightTheme.isPaleMoon ? "active" : ""
                activeOpacity: 1.0
                inactiveOpacity: 0.78
                activeColor: LateNightTheme.activePlayCueColor
                inactiveColor: LateNightTheme.deckDimButtonInactiveColor
                pressedActivatesFill: true
            }

            LateNightControlButton {
                Layout.preferredWidth: 26
                Layout.preferredHeight: 26
                backgroundSource: LateNightTheme.lateNightSubRegionButton("square")
                iconSource: LateNightTheme.assetDeckLoopInButton
                group: root.group
                key: "loop_in"
                rightClickKey: "loop_in_goto"
                activeBackgroundSuffix: "active"
                pressedBackgroundSuffix: "active"
                activeIconSuffix: LateNightTheme.isPaleMoon ? "active" : ""
                pressedIconSuffix: LateNightTheme.isPaleMoon ? "active" : ""
                activeOpacity: 1.0
                inactiveOpacity: 0.78
                activeColor: LateNightTheme.keyControlsPressedColor
                inactiveColor: LateNightTheme.deckDimButtonInactiveColor
                pressedActivatesFill: true
            }

            LateNightControlButton {
                Layout.preferredWidth: 26
                Layout.preferredHeight: 26
                backgroundSource: LateNightTheme.lateNightSubRegionButton("square")
                iconSource: LateNightTheme.assetDeckLoopOutButton
                group: root.group
                key: "loop_out"
                rightClickKey: "loop_out_goto"
                activeBackgroundSuffix: "active"
                pressedBackgroundSuffix: "active"
                activeIconSuffix: LateNightTheme.isPaleMoon ? "active" : ""
                pressedIconSuffix: LateNightTheme.isPaleMoon ? "active" : ""
                activeOpacity: 1.0
                inactiveOpacity: 0.78
                activeColor: LateNightTheme.keyControlsPressedColor
                inactiveColor: LateNightTheme.deckDimButtonInactiveColor
                pressedActivatesFill: true
            }

            LateNightControlButton {
                Layout.preferredWidth: 26
                Layout.preferredHeight: 26
                backgroundSource: LateNightTheme.lateNightSubRegionButton("square")
                iconSource: isActive
                        ? LateNightTheme.assetDeckLoopAnchorEndButton
                        : LateNightTheme.assetDeckLoopAnchorStartButton
                group: root.group
                key: "loop_anchor"
                toggleable: true
                activeBackgroundSuffix: "set"
                pressedBackgroundSuffix: "active"
                activeOpacity: 1.0
                inactiveOpacity: 0.78
                activeColor: LateNightTheme.deckDimButtonInactiveColor
                pressedColor: LateNightTheme.keyControlsPressedColor
                inactiveColor: LateNightTheme.deckDimButtonInactiveColor
                pressedActivatesFill: true
            }
        }
        Item {
            Layout.preferredWidth: 2
            visible: root.showLoopControls || root.showBeatjumpControls
        }
        Item {
            Layout.fillWidth: true
            Layout.maximumWidth: 80
            visible: root.showLoopControls || root.showBeatjumpControls
        }

        // Beatjump controls
        GridLayout {
            Layout.preferredHeight: 52
            Layout.preferredWidth: 60
            columnSpacing: 0
            columns: 2
            rowSpacing: 0
            rows: 2
            visible: root.showBeatjumpControls

            LateNightBeatSpinBox {
                Layout.columnSpan: 2
                Layout.preferredHeight: 26
                Layout.preferredWidth: 60
                preferredWidth: 60
                group: root.group
                key: "beatjump_size"
                decrementKey: "beatjump_size_halve"
                incrementKey: "beatjump_size_double"
            }
            LateNightControlButton {
                Layout.preferredWidth: 26
                Layout.preferredHeight: 26
                backgroundSource: LateNightTheme.lateNightSubRegionButton("square")
                iconSource: LateNightTheme.assetDeckBeatjumpLeftButton
                group: root.group
                key: "beatjump_backward"
                rightClickKey: "beatjump_1_backward"
                activeBackgroundSuffix: "active"
                pressedBackgroundSuffix: "active"
                activeIconSuffix: LateNightTheme.isPaleMoon ? "active" : ""
                pressedIconSuffix: LateNightTheme.isPaleMoon ? "active" : ""
                activeOpacity: 1.0
                inactiveOpacity: 0.82
                activeColor: LateNightTheme.keyControlsPressedColor
                inactiveColor: LateNightTheme.deckDimButtonInactiveColor
                pressedActivatesFill: true
            }

            LateNightControlButton {
                Layout.preferredWidth: 26
                Layout.preferredHeight: 26
                backgroundSource: LateNightTheme.lateNightSubRegionButton("square")
                iconSource: LateNightTheme.assetDeckBeatjumpRightButton
                group: root.group
                key: "beatjump_forward"
                rightClickKey: "beatjump_1_forward"
                activeBackgroundSuffix: "active"
                pressedBackgroundSuffix: "active"
                activeIconSuffix: LateNightTheme.isPaleMoon ? "active" : ""
                pressedIconSuffix: LateNightTheme.isPaleMoon ? "active" : ""
                activeOpacity: 1.0
                inactiveOpacity: 0.82
                activeColor: LateNightTheme.keyControlsPressedColor
                inactiveColor: LateNightTheme.deckDimButtonInactiveColor
                pressedActivatesFill: true
            }
        }
        Item {
            Layout.fillWidth: true
        }
    }
}
