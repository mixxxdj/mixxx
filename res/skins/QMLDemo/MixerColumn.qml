import Mixxx.Controls 0.1 as MixxxControls
import QtQuick 2.12
import QtQuick.Layouts 1.11

Item {
    id: root

    required property string group

    implicitWidth: 47
    implicitHeight: 150

    ColumnLayout {
        MixxxControls.Knob {
            id: gainKnob

            Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
            width: 35
            height: width
            group: root.group
            key: "pregain"
            arc: true
            arcRadius: 15
            arcColor: "#b96300"
            arcWidth: 2

            background: Image {
                source: "../LateNight/palemoon/knobs/knob_bg_master.svg"
                width: gainKnob.width
                height: gainKnob.width / 7 * 6
            }

            foreground: Image {
                source: "../LateNight/palemoon/knobs/knob_indicator_regular_orange.svg"
                width: gainKnob.width
                height: gainKnob.width / 7 * 6
            }

        }

        MixxxControls.Slider {
            id: volumeSlider

            Layout.alignment: Qt.AlignHCenter | Qt.AlignBottom
            width: root.width
            height: 107
            group: root.group
            key: "volume"
            bar: true
            barColor: "#257b82"
            barMargin: 10

            handle: Image {
                source: "../LateNight/palemoon/sliders/knob_volume_deck.svg"
                width: 42
                height: 19
            }

            background: Image {
                width: volumeSlider.width
                height: volumeSlider.height
                source: "../LateNight/palemoon/sliders/slider_volume_deck.svg"
            }

        }

    }

}
