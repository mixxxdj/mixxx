import Mixxx.Controls 0.1 as MixxxControls
import QtQuick 2.12

Item {
    id: root

    required property string channelGroup
    property alias key: knob.key

    width: 35
    height: 35

    MixxxControls.Knob {
        id: knob

        anchors.fill: parent
        group: "[EqualizerRack1_" + channelGroup + "_Effect1]"
        arc: true
        arcRadius: 15
        arcColor: "#858585"
        arcWidth: 2

        background: Image {
            source: "../LateNight/palemoon/knobs/knob_bg_master.svg"
            width: 35
            height: 30
        }

        foreground: Image {
            source: "../LateNight/palemoon/knobs/knob_indicator_regular_red.svg"
            width: 35
            height: 30
        }

    }

}
