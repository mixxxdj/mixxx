pragma ComponentBehavior: Bound

import ".." as Skin
import Mixxx 1.0 as Mixxx
import QtQuick
import "../Theme"

Row {
    id: root

    property color activeColor: Theme.effectUnitColor
    required property string channelGroup
    property color inactiveColor: Theme.knobBackgroundColor
    property bool showFourUnits: showFourUnitsControl.value > 0

    Repeater {
        model: root.showFourUnits ? 4 : 2

        Skin.ControlButton {
            required property int index

            activeColor: root.activeColor
            group: "[EffectRack1_EffectUnit" + (index + 1) + "]"
            height: 20
            key: "group_" + root.channelGroup + "_enable"
            text: root.showFourUnits ? (index === 0 ? "FX1" : index + 1) : "FX" + (index + 1)
            toggleable: true
            width: root.showFourUnits ? (index === 0 ? 26 : 20) : 26
        }
    }
    Mixxx.ControlProxy {
        id: showFourUnitsControl

        group: "[Skin]"
        key: "show_4effectunits"
    }
}
