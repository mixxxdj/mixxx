/*
This module is used to define the center right section, above the waveform.
Currently this section is dedicated to display loop state information such as loop state, anchor mode or size.
*/
import QtQuick 2.14
import QtQuick.Controls 2.15

import Mixxx 1.0 as Mixxx
import Mixxx.Controls 1.0 as MixxxControls

Rectangle {
    id: root

    required property string group

    property color loopReverseOffBoxColor: Qt.rgba(255/255,113/255,9/255, 1)
    property color loopOffBoxColor: Qt.rgba(67/255,70/255,66/255, 1)
    property color loopOffFontColor: "white"
    property color loopOnBoxColor: Qt.rgba(125/255,246/255,64/255, 1)
    property color loopOnFontColor: "black"

    Mixxx.ControlProxy {
        id: beatloopSize
        group: root.group
        key: "beatloop_size"
    }

    Mixxx.ControlProxy {
        id: loopEnabled
        group: root.group
        key: "loop_enabled"
    }

    Mixxx.ControlProxy {
        id: loopAnchor
        group: root.group
        key: "loop_anchor"
    }

    readonly property bool on: loopEnabled.value

    radius: 6
    border.width: 2
    border.color: (loopSizeIndicator.on ? loopOnBoxColor : (loopAnchor.value == 0 ? loopOffBoxColor : loopReverseOffBoxColor))
    color: (loopSizeIndicator.on ? loopOnBoxColor : (loopAnchor.value == 0 ? loopOffBoxColor : loopReverseOffBoxColor))

    Text {
        id: indicator
        text: (beatloopSize.value < 1 ? `1/${1 / beatloopSize.value}` : `${beatloopSize.value}`);
        anchors.centerIn: parent
        font.pixelSize: 46
        color: (loopSizeIndicator.on ? loopOnFontColor : loopOffFontColor)
    }

    states: State {
        name: "compacted"

        PropertyChanges {
            target: indicator
            font.pixelSize: 17
        }
    }
}
