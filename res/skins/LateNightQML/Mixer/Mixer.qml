import Mixxx 1.0 as Mixxx
import QtQuick
import "../LateNightTheme"

Item {
    id: root

    required property var groups
    property bool show4decks: false

    implicitHeight: decks.implicitHeight
    implicitWidth: decks.implicitWidth + (showMainHeadMixerControl.value > 0 ? 98 : 0)

    Rectangle {
        anchors.fill: parent
        color: LateNightTheme.mixerPanelColor
    }
    Rectangle {
        anchors.fill: parent
        border.color: LateNightTheme.mixerPanelBorderLight
        border.width: 1
        color: "transparent"
    }
    MixerDecks {
        id: decks

        groups: root.groups
        show4decks: root.show4decks
        showEqKillButtons: showEqKillButtonsControl.value > 0
        showEqKnobs: showEqKnobsControl.value > 0
        showXfader: showXfaderControl.value > 0
        x: 0
        y: 0
    }
    Rectangle {
        height: parent.height
        visible: showMainHeadMixerControl.value > 0
        width: 2
        x: decks.implicitWidth

        gradient: Gradient {
            GradientStop {
                color: LateNightTheme.mixerPanelBorderDark
                position: 0
            }
            GradientStop {
                color: LateNightTheme.mixerPanelBorderLight
                position: 1
            }
        }
    }
    MainHeadphonePanel {
        height: decks.implicitHeight
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
