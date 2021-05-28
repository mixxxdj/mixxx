import Mixxx.Controls 0.1 as MixxxControls
import QtQuick 2.12
import QtQuick.Layouts 1.11

Item {
    id: root

    required property string group

    implicitWidth: 35
    implicitHeight: 150

    ColumnLayout {
        EqKnob {
            channelGroup: root.group
            key: "parameter3"
        }

        EqKnob {
            channelGroup: root.group
            key: "parameter2"
        }

        EqKnob {
            channelGroup: root.group
            key: "parameter1"
        }

        MixxxControls.Knob {
            id: filterKnob

            width: 35
            height: width
            group: "[QuickEffectRack1_" + root.group + "]"
            key: "super1"
            arc: true
            arcRadius: 15
            arcColor: "#518f00"
            arcWidth: 2

            background: Image {
                source: "../LateNight/palemoon/knobs/knob_bg_master.svg"
                width: filterKnob.width
                height: filterKnob.width / 7 * 6
            }

            foreground: Image {
                source: "../LateNight/palemoon/knobs/knob_indicator_regular_green.svg"
                width: filterKnob.width
                height: filterKnob.width / 7 * 6
            }

        }

    }

}
