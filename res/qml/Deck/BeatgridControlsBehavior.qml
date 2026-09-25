import Mixxx 1.0 as Mixxx
import QtQuick 2.12

Item {
    id: root

    required property string group
    readonly property bool bpmLocked: bpmlockProxy.value > 0
    readonly property bool beatsUndoPossible: beatsUndoPossibleProxy.value > 0
    readonly property bool timingShiftButtonsVisible: timingShiftButtonsProxy.value > 0

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
}
