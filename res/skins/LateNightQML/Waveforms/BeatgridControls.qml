pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import Mixxx 1.0 as Mixxx
import "../LateNightTheme"
import "../Deck"

RowLayout {
    id: root

    required property string group

    Mixxx.ControlProxy {
        id: bpmlockProxy
        group: root.group
        key: "bpmlock"
    }

    Mixxx.ControlProxy {
        id: timingShiftButtonsProxy
        group: "[Skin]"
        key: "timing_shift_buttons"
    }

    Mixxx.ControlProxy {
        id: beatsUndoPossibleProxy
        group: root.group
        key: "beats_undo_possible"
    }

    spacing: 0
    Layout.fillHeight: true

    // Column 1: CurPos (26x52)
    Item {
        Layout.preferredWidth: 26
        Layout.preferredHeight: 52

        LateNightControlButton {
            anchors.fill: parent
            backgroundSource: LateNightTheme.legacyTopRegionButton("library_tall")
            iconSource: LateNightTheme.assetDeckBeatCurposLargeButton
            group: root.group
            key: "beats_translate_curpos"
            rightClickKey: "beats_translate_match_alignment"
            activeBackgroundSuffix: "active"
            pressedBackgroundSuffix: "active"
            activeOpacity: 1.0
            inactiveOpacity: 0.82
            activeColor: LateNightTheme.deckDimButtonInactiveColor
            inactiveColor: LateNightTheme.deckDimButtonInactiveColor
            enabled: bpmlockProxy.value === 0
        }

        Rectangle {
            anchors.fill: parent
            color: "#b4151517" // rgba(21, 21, 23, 180)
            visible: bpmlockProxy.value > 0
            radius: 2
        }
    }

    // Column 2: BeatsEarlier / BeatsFaster
    Item {
        Layout.preferredWidth: 26
        Layout.preferredHeight: 52

        Column {
            anchors.fill: parent
            spacing: 0

            LateNightControlButton {
                width: 26
                height: 26
                backgroundSource: LateNightTheme.legacySubRegionButton("square")
                iconSource: LateNightTheme.assetDeckBeatsEarlierButton
                group: root.group
                key: "beats_translate_earlier"
                rightClickKey: "beats_translate_half"
                activeBackgroundSuffix: "active"
                pressedBackgroundSuffix: "active"
                activeIconSuffix: LateNightTheme.isPaleMoon ? "active" : ""
                pressedIconSuffix: LateNightTheme.isPaleMoon ? "active" : ""
                activeOpacity: 1.0
                inactiveOpacity: 0.82
                activeColor: LateNightTheme.deckDimButtonInactiveColor
                inactiveColor: LateNightTheme.deckDimButtonInactiveColor
                enabled: bpmlockProxy.value === 0
            }

            LateNightControlButton {
                width: 26
                height: 26
                backgroundSource: LateNightTheme.legacySubRegionButton("square")
                iconSource: LateNightTheme.assetDeckBeatsFasterButton
                group: root.group
                key: "beats_adjust_faster"
                activeBackgroundSuffix: "active"
                pressedBackgroundSuffix: "active"
                activeIconSuffix: LateNightTheme.isPaleMoon ? "active" : ""
                pressedIconSuffix: LateNightTheme.isPaleMoon ? "active" : ""
                activeOpacity: 1.0
                inactiveOpacity: 0.82
                activeColor: LateNightTheme.deckDimButtonInactiveColor
                inactiveColor: LateNightTheme.deckDimButtonInactiveColor
                enabled: bpmlockProxy.value === 0
            }
        }

        Rectangle {
            anchors.fill: parent
            color: "#b4151517"
            visible: bpmlockProxy.value > 0
            radius: 2
        }
    }

    // Column 3: BeatsLater / BeatsSlower
    Item {
        Layout.preferredWidth: 26
        Layout.preferredHeight: 52

        Column {
            anchors.fill: parent
            spacing: 0

            LateNightControlButton {
                width: 26
                height: 26
                backgroundSource: LateNightTheme.legacySubRegionButton("square")
                iconSource: LateNightTheme.assetDeckBeatsLaterButton
                group: root.group
                key: "beats_translate_later"
                rightClickKey: "beats_translate_half"
                activeBackgroundSuffix: "active"
                pressedBackgroundSuffix: "active"
                activeIconSuffix: LateNightTheme.isPaleMoon ? "active" : ""
                pressedIconSuffix: LateNightTheme.isPaleMoon ? "active" : ""
                activeOpacity: 1.0
                inactiveOpacity: 0.82
                activeColor: LateNightTheme.deckDimButtonInactiveColor
                inactiveColor: LateNightTheme.deckDimButtonInactiveColor
                enabled: bpmlockProxy.value === 0
            }

            LateNightControlButton {
                width: 26
                height: 26
                backgroundSource: LateNightTheme.legacySubRegionButton("square")
                iconSource: LateNightTheme.assetDeckBeatsSlowerButton
                group: root.group
                key: "beats_adjust_slower"
                activeBackgroundSuffix: "active"
                pressedBackgroundSuffix: "active"
                activeIconSuffix: LateNightTheme.isPaleMoon ? "active" : ""
                pressedIconSuffix: LateNightTheme.isPaleMoon ? "active" : ""
                activeOpacity: 1.0
                inactiveOpacity: 0.82
                activeColor: LateNightTheme.deckDimButtonInactiveColor
                inactiveColor: LateNightTheme.deckDimButtonInactiveColor
                enabled: bpmlockProxy.value === 0
            }
        }

        Rectangle {
            anchors.fill: parent
            color: "#b4151517"
            visible: bpmlockProxy.value > 0
            radius: 2
        }
    }

    // Column 4: Undo / BpmLockToggle
    Column {
        Layout.preferredWidth: 26
        Layout.preferredHeight: 52
        spacing: 0

        Item {
            width: 26
            height: 26

            LateNightControlButton {
                anchors.fill: parent
                backgroundSource: LateNightTheme.legacySubRegionButton("square")
                iconSource: LateNightTheme.assetDeckUndoButton
                group: root.group
                key: "beats_undo_adjustment"
                activeBackgroundSuffix: "active"
                pressedBackgroundSuffix: "active"
                activeIconSuffix: LateNightTheme.isPaleMoon ? "active" : ""
                pressedIconSuffix: LateNightTheme.isPaleMoon ? "active" : ""
                activeOpacity: 1.0
                inactiveOpacity: 0.82
                activeColor: LateNightTheme.deckDimButtonInactiveColor
                inactiveColor: LateNightTheme.deckDimButtonInactiveColor
                enabled: beatsUndoPossibleProxy.value > 0
            }

            Rectangle {
                anchors.fill: parent
                color: "#b4151517"
                visible: beatsUndoPossibleProxy.value === 0
                radius: 2
            }
        }

        LateNightControlButton {
            width: 26
            height: 26
            backgroundSource: LateNightTheme.legacySubRegionButton("square")
            iconSource: isActive ? LateNightTheme.assetDeckBpmLockedButton : LateNightTheme.assetDeckBpmUnlockedButton
            group: root.group
            key: "bpmlock"
            toggleable: true
            activeBackgroundSuffix: "active"
            pressedBackgroundSuffix: "active"
            activeOpacity: 1.0
            inactiveOpacity: 0.82
            activeColor: LateNightTheme.deckDimButtonInactiveColor
            inactiveColor: LateNightTheme.deckDimButtonInactiveColor
        }
    }

    // Column 5 (optional): HotcuesEarlier / HotcuesLater
    Column {
        Layout.preferredWidth: 26
        Layout.preferredHeight: 52
        spacing: 0
        visible: timingShiftButtonsProxy.value > 0

        LateNightControlButton {
            width: 26
            height: 26
            backgroundSource: LateNightTheme.legacySubRegionButton("square")
            iconSource: LateNightTheme.assetDeckBeatsHotcuesEarlierButton
            group: root.group
            key: "shift_cues_earlier"
            rightClickKey: "shift_cues_earlier_small"
            activeBackgroundSuffix: "active"
            pressedBackgroundSuffix: "active"
            activeIconSuffix: LateNightTheme.isPaleMoon ? "active" : ""
            pressedIconSuffix: LateNightTheme.isPaleMoon ? "active" : ""
            activeOpacity: 1.0
            inactiveOpacity: 0.82
            activeColor: LateNightTheme.deckDimButtonInactiveColor
            inactiveColor: LateNightTheme.deckDimButtonInactiveColor
        }

        LateNightControlButton {
            width: 26
            height: 26
            backgroundSource: LateNightTheme.legacySubRegionButton("square")
            iconSource: LateNightTheme.assetDeckBeatsHotcuesLaterButton
            group: root.group
            key: "shift_cues_later"
            rightClickKey: "shift_cues_later_small"
            activeBackgroundSuffix: "active"
            pressedBackgroundSuffix: "active"
            activeIconSuffix: LateNightTheme.isPaleMoon ? "active" : ""
            pressedIconSuffix: LateNightTheme.isPaleMoon ? "active" : ""
            activeOpacity: 1.0
            inactiveOpacity: 0.82
            activeColor: LateNightTheme.deckDimButtonInactiveColor
            inactiveColor: LateNightTheme.deckDimButtonInactiveColor
        }
    }
}
