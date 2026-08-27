pragma ComponentBehavior: Bound

import QtQuick
import Mixxx 1.0 as Mixxx
import "../LateNightTheme"

Item {
    id: root

    required property string group

    Mixxx.ControlProxy {
        id: showBeatgridControlsProxy
        group: "[Skin]"
        key: "show_beatgrid_controls"
    }

    Mixxx.ControlProxy {
        id: timingShiftButtonsProxy
        group: "[Skin]"
        key: "timing_shift_buttons"
    }

    LateNightWaveformDisplay {
        id: waveformDisplay
        group: root.group
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.right: beatgridControls.visible ? beatgridControls.left : parent.right
    }

    BeatgridControls {
        id: beatgridControls
        group: root.group
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        width: timingShiftButtonsProxy.value > 0 ? 130 : 104
        visible: showBeatgridControlsProxy.value > 0
    }
}
