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

    signal updated

    Text {
        id: indicator
        text: "-"
        font.pixelSize: 17
        color: fontColor
        anchors.centerIn: parent

        Mixxx.ControlProxy {
            group: root.group
            key: "bpm"
            onValueChanged: (value) => {
                const newValue = value.toFixed(2);
                if (newValue === indicator.text) return;
                indicator.text = newValue;
                root.updated()
            }
        }
    }

    Text {
        id: range
        font.pixelSize: 9
        color: fontColor
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.rightMargin: 5
        anchors.topMargin: 2

        horizontalAlignment: Text.AlignHCenter

        Mixxx.ControlProxy {
            group: root.group
            key: "rateRange"
            onValueChanged: (value) => {
                const newValue = `-/+ \n${(value * 100).toFixed()}%`;
                if (range.text === newValue) return;
                range.text = newValue;
                root.updated();
            }
        }
    }

    states: State {
        name: "compacted"

        PropertyChanges {
            target: range
            visible: false
        }
    }
}
