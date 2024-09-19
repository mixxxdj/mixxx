/*
This module is used to define the top right section, right under the label.
Currently this section is dedicated to BPM and tempo fader information.
*/
import QtQuick 2.14
import QtQuick.Controls 2.15

import Mixxx 1.0 as Mixxx
import Mixxx.Controls 1.0 as MixxxControls

Rectangle {
    id: root

    required property string group
    required property color borderColor

    property real value: 0

    color: "transparent"
    radius: 6
    border.color: smallBoxBorder
    border.width: 2

    Mixxx.ControlProxy {
        id: bpm
        group: root.group
        key: "bpm"
    }

    Mixxx.ControlProxy {
        id: rateRange
        group: root.group
        key: "rateRange"
    }

    Text {
        id: indicator
        text: bpm.value > 0 ? bpm.value.toFixed(2) : "-"
        font.pixelSize: 17
        color: fontColor
        anchors.centerIn: parent
    }

    Text {
        id: range

        text: rateRange.value > 0 ? `-/+ \n${(rateRange.value * 100).toFixed()}%` : ''
        font.pixelSize: 9
        color: fontColor

        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.rightMargin: 5
        anchors.topMargin: 2

        horizontalAlignment: Text.AlignHCenter
    }

    states: State {
        name: "compacted"

        PropertyChanges {
            target: range
            visible: false
        }
    }
}
