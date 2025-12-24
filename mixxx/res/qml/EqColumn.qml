import "." as Skin
import QtQuick 2.12
import QtQuick.Shapes 1.12
import QtQuick.Layouts
import Mixxx 1.0 as Mixxx
import "Theme"

Column {
    id: root

    required property string group
    property var player: Mixxx.PlayerManager.getPlayer(root.group)

    Mixxx.ControlProxy {
        id: stemCountControl

        group: root.group
        key: "stem_count"
    }

    function stemGroup(group, index) {
        return `${group.substr(0, group.length-1)}_Stem${index + 1}]`
    }

    Row {
        Column {
            id: stem
            spacing: 4
            width: 10
            visible: opacity != 0
            Repeater {
                model: root.player.stemsModel

                Skin.StemKnob {
                    required property int index

                    id: stem
                    stemGroup: root.stemGroup(root.group, index)
                    property alias color: stem.stemColor
                }
            }
        }
        Column {
            id: eq
            spacing: 4
            width: 10
            visible: opacity != 0
            Skin.EqKnob {
                statusKey: "button_parameter3"
                knob.group: "[EqualizerRack1_" + root.group + "_Effect1]"
                knob.key: "parameter3"
                knob.color: Theme.eqHighColor
            }

            Skin.EqKnob {
                statusKey: "button_parameter2"
                knob.group: "[EqualizerRack1_" + root.group + "_Effect1]"
                knob.key: "parameter2"
                knob.color: Theme.eqMidColor
            }

            Skin.EqKnob {
                knob.group: "[EqualizerRack1_" + root.group + "_Effect1]"
                knob.key: "parameter1"
                statusKey: "button_parameter1"
                knob.color: Theme.eqLowColor
            }

            Skin.QuickFxKnob {
                group: "[QuickEffectRack1_" + root.group + "]"
                knob.arcStyle: ShapePath.DashLine
                knob.arcStylePattern: [2, 2]
                knob.color: Theme.eqFxColor
            }
        }
        states: [
            State {
                name: "eq"
                when: stemCountControl.value == 0
                PropertyChanges { target: stem; opacity: 0; width: 0}
            },
            State {
                name: "stem"
                when: stemCountControl.value != 0
                PropertyChanges { target: eq; opacity: 0; width: 0 }
            }
        ]

        transitions: [
            Transition {
                from: "eq"
                to: "stem"
                ParallelAnimation {
                    PropertyAnimation { targets: [eq, stem]; properties: "opacity,width"; duration: 1000}
                }
            },
            Transition {
                from: "stem"
                to: "eq"
                ParallelAnimation {
                    PropertyAnimation { targets: [eq, stem]; properties: "opacity,width"; duration: 1000}
                }
            }
        ]
    }

    Skin.OrientationToggleButton {
        group: root.group
        key: "orientation"
        color: Theme.crossfaderOrientationColor
    }
}
