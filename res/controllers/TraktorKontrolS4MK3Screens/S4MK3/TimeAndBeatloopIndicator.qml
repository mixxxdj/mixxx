/*
This module is used to define the center left section, above the waveform.
Currently this section is dedicated to show the remaining time as well as the beatloop when changing.
*/
import QtQuick 2.14
import QtQuick.Controls 2.15

import Mixxx 1.0 as Mixxx
import Mixxx.Controls 1.0 as MixxxControls

Rectangle {
    id: root

    required property string group

    property color timeColor: Qt.rgba(67/255,70/255,66/255, 1)
    property color beatjumpColor: 'yellow'

    enum Mode {
        RemainingTime,
        BeetjumpSize
    }

    property int mode: TimeAndBeatloopIndicator.Mode.RemainingTime

    radius: 6
    border.color: timeColor
    border.width: 2
    color: timeColor

    Text {
        id: indicator
        anchors.centerIn: parent
        text: "0.00"

        font.pixelSize: 46
        color: fontColor

        Mixxx.ControlProxy {
            id: progression
            group: root.group
            key: "playposition"
        }

        Mixxx.ControlProxy {
            id: duration
            group: root.group
            key: "duration"
        }

        Mixxx.ControlProxy {
            id: beatjump
            group: root.group
            key: "beatjump_size"
        }

        Mixxx.ControlProxy {
            id: endoftrack
            group: root.group
            key: "end_of_track"
            onValueChanged: (value) => {
                root.border.color = value ? 'red' : timeColor
                root.color = value ? 'red' : timeColor
            }
        }
    }

    Component.onCompleted: {
        indicator.text = Qt.binding(function() {
                let newValue = "";
                if (root.mode === TimeAndBeatloopIndicator.Mode.RemainingTime) {
                    var seconds = ((1.0 - progression.value) * duration.value);
                    newValue = `-${parseInt(seconds / 60).toString().padStart(2, '0')}:${parseInt(seconds % 60).toString().padStart(2, '0')}`;
                } else {
                    newValue = (beatjump.value < 1 ? `1/${1 / beatjump.value}` : `${beatjump.value}`);
                }
                return newValue
        });
    }

    states: State {
        name: "compacted"

        PropertyChanges {
            target: indicator
            font.pixelSize: 17
        }
    }

    onModeChanged: () => {
        border.color = root.mode == TimeAndBeatloopIndicator.Mode.BeetjumpSize ? beatjumpColor : timeColor
        color = root.mode == TimeAndBeatloopIndicator.Mode.BeetjumpSize ? beatjumpColor : timeColor
        indicator.color = root.mode == TimeAndBeatloopIndicator.Mode.BeetjumpSize ? 'black' : 'white'
    }
}
