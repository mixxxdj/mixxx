import Mixxx 1.0 as Mixxx
import QtQuick
import "../LateNightTheme"

Item {
    id: root

    required property var groups
    property bool show4decks: false

    implicitHeight: Math.max(decks.implicitHeight, showMainHeadMixerControl.value > 0 ? mainHeadphonePanel.implicitHeight : 0)
    implicitWidth: decks.implicitWidth + (showMainHeadMixerControl.value > 0 ? 98 : 0)

    Rectangle {
        anchors.fill: parent
        color: LateNightTheme.mixerPanelColor
    }
    Rectangle {
        color: LateNightTheme.mixerPanelBorderTop
        height: 1
        width: parent.width
    }
    Rectangle {
        color: LateNightTheme.mixerPanelBorderLeft
        height: parent.height
        width: 1
    }
    Rectangle {
        anchors.bottom: parent.bottom
        color: LateNightTheme.mixerPanelBorderBottom
        height: 1
        width: parent.width
    }
    Rectangle {
        anchors.right: parent.right
        color: LateNightTheme.mixerPanelBorderRight
        height: parent.height
        width: 1
    }
    MixerDecks {
        id: decks

        groups: root.groups
        height: root.implicitHeight
        show4decks: root.show4decks
        showEqKillButtons: showEqKillButtonsControl.value > 0
        showEqKnobs: showEqKnobsControl.value > 0
        showXfader: showXfaderControl.value > 0
        x: 0
        y: 0
    }
    Item {
        height: parent.height
        visible: showMainHeadMixerControl.value > 0
        width: 2
        x: decks.implicitWidth

        Rectangle {
            anchors.left: parent.left
            color: LateNightTheme.mixerMainSeparatorDarkColor
            height: parent.height
            width: 1
        }
        Rectangle {
            anchors.right: parent.right
            color: LateNightTheme.mixerMainSeparatorLightColor
            height: parent.height
            width: 1
        }
    }
    MainHeadphonePanel {
        id: mainHeadphonePanel

        height: root.implicitHeight
        show4decks: root.show4decks
        visible: showMainHeadMixerControl.value > 0
        width: 96
        x: decks.implicitWidth + 2
        y: 0
    }
    Mixxx.ControlProxy {
        id: showEqKnobsControl

        group: "[Skin]"
        key: "show_eq_knobs"
    }
    Mixxx.ControlProxy {
        id: showEqKillButtonsControl

        group: "[Skin]"
        key: "show_eq_kill_buttons"
    }
    Mixxx.ControlProxy {
        id: showXfaderControl

        group: "[Skin]"
        key: "show_xfader"
    }
    Mixxx.ControlProxy {
        id: showMainHeadMixerControl

        group: "[Skin]"
        key: "show_main_head_mixer"
    }
}
