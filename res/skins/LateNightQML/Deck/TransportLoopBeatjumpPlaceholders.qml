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

    height: 55
    clip: true

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

    function beatSizeText(value) {
        if (value >= 1) {
            return value.toFixed(0);
        }
        return value.toString();
    }

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
            color: LateNightTheme.deckPanelBorderDark
        }

        Rectangle {
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: 1
            color: LateNightTheme.deckPanelBorderDark
        }

        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            height: 1
            color: LateNightTheme.deckPanelBorderLight
        }

        Rectangle {
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: 1
            color: LateNightTheme.deckPanelBorderLight
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
            //              Display from play_indicator
            LateNightControlButton {
                Layout.columnSpan: 2
                Layout.preferredWidth: 68
                Layout.preferredHeight: 26
                backgroundSource: LateNightTheme.lateNightSubRegionButton("play")
                iconSource: LateNightTheme.assetDeckPlayButton
                group: root.group
                key: "play"
                rightClickKey: "cue_set"
                displayKey: "play_indicator"
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

        // Hotcue controls (behavior in progress)
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

                delegate: LateNightIconButton {
                    Layout.preferredWidth: 26
                    Layout.preferredHeight: 26
                    iconSource: LateNightTheme.lateNightButton("btn__" + (index + 1) + ".svg")
                    contentOpacity: 1.0
                    inactiveColor: LateNightTheme.deckEmbeddedButtonInactiveColor
                }
            }
        }

        Item {
            Layout.preferredWidth: 4
        }

        // Intro/Outro controls (behavior in progress)
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
                    LateNightTheme.assetDeckIntroStartButton,
                    LateNightTheme.assetDeckIntroEndButton,
                    LateNightTheme.assetDeckOutroStartButton,
                    LateNightTheme.assetDeckOutroEndButton
                ]

                delegate: LateNightIconButton {
                    Layout.preferredWidth: 26
                    Layout.preferredHeight: 26
                    iconSource: modelData
                    contentOpacity: 0.72
                    inactiveColor: LateNightTheme.deckEmbeddedButtonInactiveColor
                }
            }
        }

        Item {
            Layout.preferredWidth: 8
        }

        // Loop controls (behavior in progress)
        GridLayout {
            columns: 4
            rows: 2
            rowSpacing: 0
            columnSpacing: 0
            Layout.preferredWidth: 104
            Layout.preferredHeight: 52
            visible: root.showLoopControls

            LateNightIconButton {
                Layout.preferredWidth: 26
                Layout.preferredHeight: 26
                iconSource: LateNightTheme.assetDeckLoopButton
                contentOpacity: 0.82
                inactiveColor: LateNightTheme.deckDimButtonInactiveColor
            }

            BeatSpinBoxPlaceholder {
                Layout.columnSpan: 3
                Layout.preferredWidth: 78
                Layout.preferredHeight: 26
                valueText: root.beatSizeText(beatloopSizeProxy.value)
            }

            Repeater {
                model: [
                    LateNightTheme.assetDeckReloopButton,
                    LateNightTheme.assetDeckLoopInButton,
                    LateNightTheme.assetDeckLoopOutButton,
                    LateNightTheme.assetDeckLoopAnchorStartButton
                ]

                delegate: LateNightIconButton {
                    Layout.preferredWidth: 26
                    Layout.preferredHeight: 26
                    iconSource: modelData
                    contentOpacity: 0.78
                    inactiveColor: LateNightTheme.deckDimButtonInactiveColor
                }
            }
        }

        Item {
            Layout.preferredWidth: 8
        }

        // Beatjump controls (behavior in progress)
        GridLayout {
            columns: 2
            rows: 2
            rowSpacing: 0
            columnSpacing: 0
            Layout.preferredWidth: 60
            Layout.preferredHeight: 52
            visible: root.showBeatjumpControls

            BeatSpinBoxPlaceholder {
                Layout.columnSpan: 2
                Layout.preferredWidth: 60
                Layout.preferredHeight: 26
                preferredWidth: 60
                valueText: root.beatSizeText(beatjumpSizeProxy.value)
            }

            LateNightIconButton {
                Layout.preferredWidth: 26
                Layout.preferredHeight: 26
                iconSource: LateNightTheme.assetDeckBeatjumpLeftButton
                contentOpacity: 0.82
                inactiveColor: LateNightTheme.deckDimButtonInactiveColor
            }

            LateNightIconButton {
                Layout.preferredWidth: 26
                Layout.preferredHeight: 26
                iconSource: LateNightTheme.assetDeckBeatjumpRightButton
                contentOpacity: 0.82
                inactiveColor: LateNightTheme.deckDimButtonInactiveColor
            }
        }

        Item {
            Layout.fillWidth: true
        }
    }
}
