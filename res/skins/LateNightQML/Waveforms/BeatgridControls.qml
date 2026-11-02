pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import "../LateNightTheme"
import "../Deck"
import "../Controls" as Controls
import "../../../qml/Deck" as SharedDeck

Controls.Panel {
    id: root

    required property string group

    clip: true
    color: LateNightTheme.waveformOverlayColor
    implicitHeight: 52
    implicitWidth: beatgridBehavior.timingShiftButtonsVisible ? 130 : 104
    bottomBorderColor: LateNightTheme.isPaleMoon ? "#020202" : "#111111"
    leftBorderColor: LateNightTheme.isPaleMoon ? "#1c1c1c" : "#222222"
    rightBorderColor: "#111111"
    topBorderColor: LateNightTheme.isPaleMoon ? "#1c1c1c" : "#222222"

    MouseArea {
        acceptedButtons: Qt.AllButtons
        anchors.fill: parent
        z: -1

        onWheel: wheel => wheel.accepted = true
    }
    SharedDeck.BeatgridControlsBehavior {
        id: beatgridBehavior

        group: root.group
    }
    RowLayout {
        anchors.centerIn: parent
        spacing: 0

        // Column 1: CurPos (26x52)
        Item {
            Layout.preferredHeight: 52
            Layout.preferredWidth: 26

            LateNightControlButton {
                activeBackgroundSuffix: "active"
                activeColor: LateNightTheme.deckDimButtonInactiveColor
                activeOpacity: 1.0
                anchors.fill: parent
                backgroundSource: LateNightTheme.lateNightTopRegionButton("library_tall")
                enabled: !beatgridBehavior.bpmLocked
                group: root.group
                iconSource: LateNightTheme.assetDeckBeatCurposLargeButton
                inactiveColor: LateNightTheme.deckDimButtonInactiveColor
                inactiveOpacity: 0.82
                key: "beats_translate_curpos"
                pressedBackgroundSuffix: "active"
                rightClickKey: "beats_translate_match_alignment"
            }
            Rectangle {
                anchors.fill: parent
                color: LateNightTheme.beatgridDisabledCoverColor
                radius: 2
                visible: beatgridBehavior.bpmLocked
            }
        }

        // Column 2: BeatsEarlier / BeatsFaster
        Item {
            Layout.preferredHeight: 52
            Layout.preferredWidth: 26

            Column {
                anchors.fill: parent
                spacing: 0

                LateNightControlButton {
                    activeBackgroundSuffix: "active"
                    activeColor: LateNightTheme.deckDimButtonInactiveColor
                    activeIconSuffix: LateNightTheme.isPaleMoon ? "active" : ""
                    activeOpacity: 1.0
                    backgroundSource: LateNightTheme.lateNightSubRegionButton("square")
                    enabled: !beatgridBehavior.bpmLocked
                    group: root.group
                    height: 26
                    iconSource: LateNightTheme.assetDeckBeatsEarlierButton
                    inactiveColor: LateNightTheme.deckDimButtonInactiveColor
                    inactiveOpacity: 0.82
                    key: "beats_translate_earlier"
                    pressedBackgroundSuffix: "active"
                    pressedIconSuffix: LateNightTheme.isPaleMoon ? "active" : ""
                    rightClickKey: "beats_translate_half"
                    width: 26
                }
                LateNightControlButton {
                    activeBackgroundSuffix: "active"
                    activeColor: LateNightTheme.deckDimButtonInactiveColor
                    activeIconSuffix: LateNightTheme.isPaleMoon ? "active" : ""
                    activeOpacity: 1.0
                    backgroundSource: LateNightTheme.lateNightSubRegionButton("square")
                    enabled: !beatgridBehavior.bpmLocked
                    group: root.group
                    height: 26
                    iconSource: LateNightTheme.assetDeckBeatsFasterButton
                    inactiveColor: LateNightTheme.deckDimButtonInactiveColor
                    inactiveOpacity: 0.82
                    key: "beats_adjust_faster"
                    pressedBackgroundSuffix: "active"
                    pressedIconSuffix: LateNightTheme.isPaleMoon ? "active" : ""
                    width: 26
                }
            }
            Rectangle {
                anchors.fill: parent
                color: LateNightTheme.beatgridDisabledCoverColor
                radius: 2
                visible: beatgridBehavior.bpmLocked
            }
        }

        // Column 3: BeatsLater / BeatsSlower
        Item {
            Layout.preferredHeight: 52
            Layout.preferredWidth: 26

            Column {
                anchors.fill: parent
                spacing: 0

                LateNightControlButton {
                    activeBackgroundSuffix: "active"
                    activeColor: LateNightTheme.deckDimButtonInactiveColor
                    activeIconSuffix: LateNightTheme.isPaleMoon ? "active" : ""
                    activeOpacity: 1.0
                    backgroundSource: LateNightTheme.lateNightSubRegionButton("square")
                    enabled: !beatgridBehavior.bpmLocked
                    group: root.group
                    height: 26
                    iconSource: LateNightTheme.assetDeckBeatsLaterButton
                    inactiveColor: LateNightTheme.deckDimButtonInactiveColor
                    inactiveOpacity: 0.82
                    key: "beats_translate_later"
                    pressedBackgroundSuffix: "active"
                    pressedIconSuffix: LateNightTheme.isPaleMoon ? "active" : ""
                    rightClickKey: "beats_translate_half"
                    width: 26
                }
                LateNightControlButton {
                    activeBackgroundSuffix: "active"
                    activeColor: LateNightTheme.deckDimButtonInactiveColor
                    activeIconSuffix: LateNightTheme.isPaleMoon ? "active" : ""
                    activeOpacity: 1.0
                    backgroundSource: LateNightTheme.lateNightSubRegionButton("square")
                    enabled: !beatgridBehavior.bpmLocked
                    group: root.group
                    height: 26
                    iconSource: LateNightTheme.assetDeckBeatsSlowerButton
                    inactiveColor: LateNightTheme.deckDimButtonInactiveColor
                    inactiveOpacity: 0.82
                    key: "beats_adjust_slower"
                    pressedBackgroundSuffix: "active"
                    pressedIconSuffix: LateNightTheme.isPaleMoon ? "active" : ""
                    width: 26
                }
            }
            Rectangle {
                anchors.fill: parent
                color: LateNightTheme.beatgridDisabledCoverColor
                radius: 2
                visible: beatgridBehavior.bpmLocked
            }
        }

        // Column 4: Undo / BpmLockToggle
        Column {
            Layout.preferredHeight: 52
            Layout.preferredWidth: 26
            spacing: 0

            Item {
                height: 26
                width: 26

                LateNightControlButton {
                    activeBackgroundSuffix: "active"
                    activeColor: LateNightTheme.deckDimButtonInactiveColor
                    activeIconSuffix: LateNightTheme.isPaleMoon ? "active" : ""
                    activeOpacity: 1.0
                    anchors.fill: parent
                    backgroundSource: LateNightTheme.lateNightSubRegionButton("square")
                    enabled: beatgridBehavior.beatsUndoPossible
                    group: root.group
                    iconSource: LateNightTheme.assetDeckUndoButton
                    inactiveColor: LateNightTheme.deckDimButtonInactiveColor
                    inactiveOpacity: 0.82
                    key: "beats_undo_adjustment"
                    pressedBackgroundSuffix: "active"
                    pressedIconSuffix: LateNightTheme.isPaleMoon ? "active" : ""
                }
                Rectangle {
                    anchors.fill: parent
                    color: LateNightTheme.beatgridDisabledCoverColor
                    radius: 2
                    visible: !beatgridBehavior.beatsUndoPossible
                }
            }
            LateNightControlButton {
                activeBackgroundSuffix: "active"
                activeColor: LateNightTheme.deckDimButtonInactiveColor
                activeOpacity: 1.0
                backgroundSource: LateNightTheme.lateNightSubRegionButton("square")
                group: root.group
                height: 26
                iconSource: isActive ? LateNightTheme.assetDeckBpmLockedButton : LateNightTheme.assetDeckBpmUnlockedButton
                inactiveColor: LateNightTheme.deckDimButtonInactiveColor
                inactiveOpacity: 0.82
                key: "bpmlock"
                pressedBackgroundSuffix: "active"
                toggleable: true
                width: 26
            }
        }

        // Column 5 (optional): HotcuesEarlier / HotcuesLater
        Column {
            Layout.preferredHeight: 52
            Layout.preferredWidth: 26
            spacing: 0
            visible: beatgridBehavior.timingShiftButtonsVisible

            LateNightControlButton {
                activeBackgroundSuffix: "active"
                activeColor: LateNightTheme.deckDimButtonInactiveColor
                activeIconSuffix: LateNightTheme.isPaleMoon ? "active" : ""
                activeOpacity: 1.0
                backgroundSource: LateNightTheme.lateNightSubRegionButton("square")
                group: root.group
                height: 26
                iconSource: LateNightTheme.assetDeckBeatsHotcuesEarlierButton
                inactiveColor: LateNightTheme.deckDimButtonInactiveColor
                inactiveOpacity: 0.82
                key: "shift_cues_earlier"
                pressedBackgroundSuffix: "active"
                pressedIconSuffix: LateNightTheme.isPaleMoon ? "active" : ""
                rightClickKey: "shift_cues_earlier_small"
                width: 26
            }
            LateNightControlButton {
                activeBackgroundSuffix: "active"
                activeColor: LateNightTheme.deckDimButtonInactiveColor
                activeIconSuffix: LateNightTheme.isPaleMoon ? "active" : ""
                activeOpacity: 1.0
                backgroundSource: LateNightTheme.lateNightSubRegionButton("square")
                group: root.group
                height: 26
                iconSource: LateNightTheme.assetDeckBeatsHotcuesLaterButton
                inactiveColor: LateNightTheme.deckDimButtonInactiveColor
                inactiveOpacity: 0.82
                key: "shift_cues_later"
                pressedBackgroundSuffix: "active"
                pressedIconSuffix: LateNightTheme.isPaleMoon ? "active" : ""
                rightClickKey: "shift_cues_later_small"
                width: 26
            }
        }
    }
}
