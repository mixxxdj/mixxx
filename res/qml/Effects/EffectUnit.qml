import ".." as Skin
import Mixxx 1.0 as Mixxx
import QtQuick
import "../Theme"

Item {
    id: root

    readonly property bool expanded: expandedControl.value > 0
    readonly property string group: unit.group
    readonly property Mixxx.EffectUnitProxy unit: Mixxx.EffectsManager.getEffectUnit(unitNumber)
    required property int unitNumber

    implicitHeight: effectContainer.height

    Item {
        id: effectContainer

        anchors.left: parent.left
        anchors.margins: 5
        anchors.right: effectUnitControlsFrame.left
        anchors.top: parent.top
        height: root.expanded ? 160 : 60

        EffectSlot {
            id: effect1

            anchors.left: parent.left
            anchors.top: parent.top
            effectNumber: 1
            expanded: root.expanded
            height: 50
            unitNumber: root.unitNumber
            width: root.expanded ? parent.width : parent.width / 3
        }
        EffectSlot {
            id: effect2

            anchors.left: root.expanded ? parent.left : effect1.right
            anchors.top: root.expanded ? effect1.bottom : parent.top
            effectNumber: 2
            expanded: root.expanded
            height: 50
            unitNumber: root.unitNumber
            width: root.expanded ? parent.width : parent.width / 3
        }
        EffectSlot {
            id: effect3

            anchors.left: root.expanded ? parent.left : effect2.right
            anchors.top: root.expanded ? effect2.bottom : parent.top
            effectNumber: 3
            expanded: root.expanded
            height: 50
            unitNumber: root.unitNumber
            width: root.expanded ? parent.width : parent.width / 3
        }
    }
    Rectangle {
        id: effectUnitControlsFrame

        anchors.bottom: parent.bottom
        anchors.margins: 5
        anchors.right: parent.right
        anchors.top: parent.top
        color: Theme.knobBackgroundColor
        radius: 5
        width: 150

        Column {
            anchors.fill: parent
            anchors.margins: 5
            spacing: 5

            Row {
                spacing: 5

                Skin.ControlButton {
                    activeColor: Theme.effectUnitColor
                    group: root.group
                    height: 26
                    key: "show_parameters"
                    text: root.expanded ? "▲" : "▼"
                    toggleable: true
                    width: 36
                }
                Skin.Button {
                    activeColor: Theme.effectUnitColor
                    height: 26
                    text: "FX " + root.unitNumber
                    width: 56

                    onClicked: presetPopup.open()
                }
                Skin.ControlButton {
                    activeColor: Theme.effectUnitColor
                    group: root.group
                    height: 26
                    key: "group_[Headphone]_enable"
                    text: "PFL"
                    toggleable: true
                    width: 42
                }
            }
            Row {
                spacing: 5

                Skin.ControlButton {
                    activeColor: Theme.effectUnitColor
                    group: root.group
                    height: 30
                    key: "mix_mode"
                    text: "D/W"
                    toggleable: true
                    width: 36
                }
                Skin.ControlKnob {
                    arcStart: Skin.Knob.ArcStart.Minimum
                    color: Theme.effectUnitColor
                    group: root.group
                    height: 40
                    key: "mix"
                    width: 40
                }
                Skin.ControlKnob {
                    arcStart: Skin.Knob.ArcStart.Minimum
                    color: Theme.effectUnitColor
                    group: root.group
                    height: 40
                    key: "super1"
                    visible: showSuperKnobsControl.value > 0
                    width: visible ? 40 : 0
                }
            }
        }
    }
    EffectChainPresetPopup {
        id: presetPopup

        slot1: effect1.slot
        slot2: effect2.slot
        slot3: effect3.slot
        unit: root.unit
    }
    Mixxx.ControlProxy {
        id: expandedControl

        group: root.group
        key: "show_parameters"
    }
    Mixxx.ControlProxy {
        id: showSuperKnobsControl

        group: "[Skin]"
        key: "show_superknobs"
    }
}
