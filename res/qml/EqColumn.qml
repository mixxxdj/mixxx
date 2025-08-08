import "." as Skin
import QtQuick 2.12
import QtQuick.Shapes 1.12
import QtQuick.Layouts
import Mixxx 1.0 as Mixxx
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

    Skin.QuickFxKnob {
        group: "[QuickEffectRack1_" + root.group + "]"
        knob.arcStyle: ShapePath.DashLine
        knob.arcStylePattern: [2, 2]
        knob.color: Theme.eqFxColor
    }
    Mixxx.ControlProxy {
        id: fxSelect

        group: "[QuickEffectRack1_" + root.group + "]"
        key: "loaded_chain_preset"
    }

    Skin.ComboBox {
        id: effectSelector
        width: parent.width
        spacing: 2
        indicator.width: 0
        popupWidth: 150
        clip: true

        textRole: "display"
        font.pixelSize: 10
        model: Mixxx.EffectsManager.quickChainPresetModel
        currentIndex: fxSelect.value == -1 ? 0 : fxSelect.value
        onActivated: (index) => {
            fxSelect.value = index
        }
    }
}
