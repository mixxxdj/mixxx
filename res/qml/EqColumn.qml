import "." as Skin
import QtQuick 2.12
import QtQuick.Shapes 1.12
import "Theme"

Column {
    id: root

    required property string group

    spacing: 4

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

    Skin.EqKnob {
        knob.group: "[QuickEffectRack1_" + root.group + "]"
        knob.key: "super1"
        statusGroup: "[QuickEffectRack1_" + root.group + "_Effect1]"
        statusKey: "enabled"
        knob.arcStyle: ShapePath.DashLine
        knob.arcStylePattern: [2, 2]
        knob.color: Theme.eqFxColor
    }

    Skin.OrientationToggleButton {
        group: root.group
        key: "orientation"
        color: Theme.crossfaderOrientationColor
    }
}
