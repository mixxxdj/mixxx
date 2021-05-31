import "." as Skin
import Mixxx.Controls 0.1 as MixxxControls
import QtQuick 2.12
import QtQuick.Shapes 1.12
import "Theme"

Column {
    id: root

    required property string group

    spacing: 4

    Knob {
        group: "[EqualizerRack1_" + root.group + "_Effect1]"
        key: "parameter3"
        color: Theme.eqHighColor
    }

    Knob {
        group: "[EqualizerRack1_" + root.group + "_Effect1]"
        key: "parameter2"
        color: Theme.eqMidColor
    }

    Knob {
        group: "[EqualizerRack1_" + root.group + "_Effect1]"
        key: "parameter1"
        color: Theme.eqLowColor
    }

    Knob {
        group: "[QuickEffectRack1_" + root.group + "]"
        arcStyle: ShapePath.DashLine
        arcStylePattern: [2, 2]
        key: "super1"
        color: Theme.eqFxColor
    }

    Skin.OrientationToggleButton {
        group: root.group
        key: "orientation"
        color: Theme.crossfaderOrientationColor
    }

}
