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

    function beatSizeText(value) {
        if (value >= 1) {
            return value.toFixed(0);
        }
        return value.toString();
    }

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
    Rectangle {
        anchors.fill: parent
        color: "#151515"
        visible: LateNightTheme.optionalDeckControlsBackgroundTile.toString().length > 0

        Image {
            anchors.fill: parent
            fillMode: Image.Tile
            source: LateNightTheme.optionalDeckControlsBackgroundTile
        }
        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            color: LateNightTheme.deckPanelBorderDark
            height: 1
        }
        Rectangle {
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.top: parent.top
            color: LateNightTheme.deckPanelBorderDark
            width: 1
        }
        Rectangle {
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            color: LateNightTheme.deckPanelBorderLight
            height: 1
        }
        Rectangle {
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            anchors.top: parent.top
            color: LateNightTheme.deckPanelBorderLight
            width: 1
        }
    }
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

            // Play button: left-click = play (toggle via play_latched),
            //              right-click = cue_set
            LateNightControlButton {
                id: playButton

                Layout.columnSpan: 2
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
        }
        Item {
            Layout.preferredWidth: 4
            visible: root.showHotcues || root.showIntroOutroCues
        }

        // Hotcue controls (behavior in progress)
        GridLayout {
            Layout.preferredHeight: 52
            Layout.preferredWidth: root.show8Hotcues ? 104 : 52
            columnSpacing: 0
            columns: root.show8Hotcues ? 4 : 2
            rowSpacing: 0
            rows: 2
            visible: root.showHotcues

            Repeater {
                model: 8

                delegate: LateNightIconButton {
                    Layout.preferredHeight: 26
                    Layout.preferredWidth: root.show8Hotcues || index < 4 ? 26 : 0
                    contentOpacity: 1.0
                    iconSource: LateNightTheme.lateNightButton("btn__" + (index + 1) + ".svg")
                    inactiveColor: LateNightTheme.deckEmbeddedButtonInactiveColor
                    visible: root.show8Hotcues || index < 4
                }
            }
        }
        Item {
            Layout.preferredWidth: 4
            visible: root.showHotcues || root.showIntroOutroCues
        }

        // Intro/Outro controls (behavior in progress)
        GridLayout {
            Layout.preferredHeight: 52
            Layout.preferredWidth: 52
            columnSpacing: 0
            columns: 2
            rowSpacing: 0
            rows: 2
            visible: root.showIntroOutroCues

            Repeater {
                model: [LateNightTheme.assetDeckIntroStartButton, LateNightTheme.assetDeckIntroEndButton, LateNightTheme.assetDeckOutroStartButton, LateNightTheme.assetDeckOutroEndButton]

                delegate: LateNightIconButton {
                    Layout.preferredHeight: 26
                    Layout.preferredWidth: 26
                    contentOpacity: 0.72
                    iconSource: modelData
                    inactiveColor: LateNightTheme.deckEmbeddedButtonInactiveColor
                }
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

        // Loop controls (behavior in progress)
        GridLayout {
            Layout.preferredHeight: 52
            Layout.preferredWidth: 104
            columnSpacing: 0
            columns: 4
            rowSpacing: 0
            rows: 2
            visible: root.showLoopControls

            LateNightIconButton {
                Layout.preferredHeight: 26
                Layout.preferredWidth: 26
                contentOpacity: 0.82
                iconSource: LateNightTheme.assetDeckLoopButton
                inactiveColor: LateNightTheme.deckDimButtonInactiveColor
            }
            BeatSpinBoxPlaceholder {
                Layout.columnSpan: 3
                Layout.preferredHeight: 26
                Layout.preferredWidth: 78
                valueText: root.beatSizeText(beatloopSizeProxy.value)
            }
            Repeater {
                model: [LateNightTheme.assetDeckReloopButton, LateNightTheme.assetDeckLoopInButton, LateNightTheme.assetDeckLoopOutButton, LateNightTheme.assetDeckLoopAnchorStartButton]

                delegate: LateNightIconButton {
                    Layout.preferredHeight: 26
                    Layout.preferredWidth: 26
                    contentOpacity: 0.78
                    iconSource: modelData
                    inactiveColor: LateNightTheme.deckDimButtonInactiveColor
                }
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

        // Beatjump controls (behavior in progress)
        GridLayout {
            Layout.preferredHeight: 52
            Layout.preferredWidth: 60
            columnSpacing: 0
            columns: 2
            rowSpacing: 0
            rows: 2
            visible: root.showBeatjumpControls

            BeatSpinBoxPlaceholder {
                Layout.columnSpan: 2
                Layout.preferredHeight: 26
                Layout.preferredWidth: 60
                preferredWidth: 60
                valueText: root.beatSizeText(beatjumpSizeProxy.value)
            }
            LateNightIconButton {
                Layout.preferredHeight: 26
                Layout.preferredWidth: 26
                contentOpacity: 0.82
                iconSource: LateNightTheme.assetDeckBeatjumpLeftButton
                inactiveColor: LateNightTheme.deckDimButtonInactiveColor
            }
            LateNightIconButton {
                Layout.preferredHeight: 26
                Layout.preferredWidth: 26
                contentOpacity: 0.82
                iconSource: LateNightTheme.assetDeckBeatjumpRightButton
                inactiveColor: LateNightTheme.deckDimButtonInactiveColor
            }
        }
        Item {
            Layout.fillWidth: true
        }
    }
}
