pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import "../LateNightTheme"

Item {
    id: root

    required property string group
    property bool show8Hotcues: true
    property bool showBeatjumpControls: true
    property bool showHotcues: true
    property bool showIntroOutroCues: true
    property bool showLoopControls: true

    height: 55
    clip: true

    Rectangle {
        anchors.fill: parent
        color: "#151515"
        visible: LateNightTheme.optionalDeckControlsBackgroundTile.toString().length > 0

        Image {
            anchors.fill: parent
            source: LateNightTheme.optionalDeckControlsBackgroundTile
            fillMode: Image.Tile
        }

        Rectangle {
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.right: parent.right
            height: 1
            color: "#0a0a0a"
        }

        Rectangle {
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: 1
            color: "#0a0a0a"
        }

        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            height: 1
            color: "#333333"
        }

        Rectangle {
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: 1
            color: "#333333"
        }
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: LateNightTheme.optionalDeckControlsBackgroundTile.toString().length > 0 ? 1 : 0
        anchors.topMargin: LateNightTheme.optionalDeckControlsBackgroundTile.toString().length > 0 ? 1 : 0
        anchors.rightMargin: LateNightTheme.optionalDeckControlsBackgroundTile.toString().length > 0 ? 1 : 0
        anchors.bottomMargin: LateNightTheme.optionalDeckControlsBackgroundTile.toString().length > 0 ? 2 : 0
        spacing: 6

        GridLayout {
            columns: 2
            rows: 2
            rowSpacing: 0
            columnSpacing: 0
            Layout.preferredWidth: 68
            Layout.preferredHeight: 52

            // Cue button: left-click = cue_default, right-click = cue_gotoandstop
            // Display from cue_indicator
            LateNightControlButton {
                Layout.preferredWidth: 42
                Layout.preferredHeight: 26
                backgroundSource: LateNightTheme.lateNightSubRegionButton("medium")
                iconSource: LateNightTheme.assetDeckCueButton
                group: root.group
                key: "cue_default"
                rightClickKey: "cue_gotoandstop"
                displayKey: "cue_indicator"
                activeOpacity: 1.0
                inactiveOpacity: 0.82
                activeBackgroundSuffix: "set"
                pressedBackgroundSuffix: "active"
                activeIconSuffix: LateNightTheme.playCueActiveIconSuffix
                pressedIconSuffix: LateNightTheme.playCueActiveIconSuffix
                activeColor: LateNightTheme.activePlayCueColor
                pressedActivatesFill: true
            }

            // Reverse button: left-click = reverse, right-click = reverseroll
            LateNightControlButton {
                Layout.preferredWidth: 26
                Layout.preferredHeight: 26
                backgroundSource: LateNightTheme.lateNightSubRegionButton("square")
                activeBackgroundSuffix: "active"
                iconSource: LateNightTheme.assetDeckReverseButton
                activeIconSuffix: LateNightTheme.playCueActiveIconSuffix
                stretchIcon: true
                group: root.group
                key: "reverse"
                rightClickKey: "reverseroll"
                activeOpacity: 1.0
                inactiveOpacity: 0.82
                activeColor: LateNightTheme.activePlayCueColor
            }

            // Play button: left-click = play (toggle via play_latched),
            //              right-click = cue_set
            //              Display from play_latched
            LateNightControlButton {
                Layout.columnSpan: 2
                Layout.preferredWidth: 68
                Layout.preferredHeight: 26
                backgroundSource: LateNightTheme.lateNightSubRegionButton("play")
                iconSource: LateNightTheme.assetDeckPlayButton
                group: root.group
                key: "play"
                rightClickKey: "cue_set"
                displayKey: "play_latched"
                toggleable: true
                activeOpacity: 1.0
                inactiveOpacity: 0.82
                activeBackgroundSuffix: "active"
                activeIconSuffix: LateNightTheme.playCueActiveIconSuffix
                activeColor: LateNightTheme.activePlayCueColor
            }
        }

        Item {
            Layout.preferredWidth: 4
        }

        // Hotcue controls
        GridLayout {
            columns: root.show8Hotcues ? 4 : 2
            rows: 2
            rowSpacing: 0
            columnSpacing: 0
            Layout.preferredWidth: root.show8Hotcues ? 104 : 52
            Layout.preferredHeight: 52
            visible: root.showHotcues

            Repeater {
                model: root.show8Hotcues ? 8 : 4

                delegate: LateNightHotcueButton {
                    required property int index

                    Layout.preferredWidth: 26
                    Layout.preferredHeight: 26
                    group: root.group
                    hotcueNumber: index + 1
                }
            }
        }

        Item {
            Layout.preferredWidth: 4
        }

        // Intro/Outro controls
        GridLayout {
            columns: 2
            rows: 2
            rowSpacing: 0
            columnSpacing: 0
            Layout.preferredWidth: 52
            Layout.preferredHeight: 52
            visible: root.showIntroOutroCues

            Repeater {
                model: [
                    "intro_start",
                    "intro_end",
                    "outro_start",
                    "outro_end"
                ]

                delegate: LateNightSpecialCueButton {
                    required property string modelData

                    Layout.preferredWidth: 26
                    Layout.preferredHeight: 26
                    group: root.group
                    cueType: modelData
                }
            }
        }

        Item {
            Layout.preferredWidth: 8
        }

        // Loop controls
        GridLayout {
            columns: 4
            rows: 2
            rowSpacing: 0
            columnSpacing: 0
            Layout.preferredWidth: 104
            Layout.preferredHeight: 52
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
                Layout.preferredWidth: 78
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
                iconSource: LateNightTheme.assetDeckLoopAnchorStartButton
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
            Layout.preferredWidth: 8
        }

        // Beatjump controls
        GridLayout {
            columns: 2
            rows: 2
            rowSpacing: 0
            columnSpacing: 0
            Layout.preferredWidth: 60
            Layout.preferredHeight: 52
            visible: root.showBeatjumpControls

            LateNightBeatSpinBox {
                Layout.columnSpan: 2
                Layout.preferredWidth: 60
                Layout.preferredHeight: 26
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
