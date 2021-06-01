import "." as Skin
import Mixxx.Controls 0.1 as MixxxControls
import QtQuick 2.12
import QtQuick.Shapes 1.12
import "Theme"

Column {
    id: root

    property string group // required

    spacing: 4

    EqKnob {
        group: "[EqualizerRack1_" + root.group + "_Effect1]"
        key: "parameter3"
        statusKey: "button_parameter3"
        color: Theme.eqHighColor
    }

    EqKnob {
        group: "[EqualizerRack1_" + root.group + "_Effect1]"
        key: "parameter2"
        statusKey: "button_parameter2"
        color: Theme.eqMidColor
    }

    EqKnob {
        group: "[EqualizerRack1_" + root.group + "_Effect1]"
        key: "parameter1"
        statusKey: "button_parameter1"
        color: Theme.eqLowColor
    }

    EqKnob {
        group: "[QuickEffectRack1_" + root.group + "]"
        key: "super1"
        statusGroup: "[QuickEffectRack1_" + root.group + "_Effect1]"
        statusKey: "enabled"
        arcStyle: ShapePath.DashLine
        arcStylePattern: [2, 2]
        color: Theme.eqFxColor
    }

    Skin.OrientationToggleButton {
        group: root.group
        key: "orientation"
        color: Theme.crossfaderOrientationColor
    }

}
