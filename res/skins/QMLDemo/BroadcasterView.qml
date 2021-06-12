import "." as Skin
import QtQuick 2.12
import "Theme"

Item {
    id: root

    Row {
        height: parent.height
        anchors.horizontalCenter: parent.horizontalCenter

        Item {
            width: 56
            height: parent.height

            Skin.VuMeter {
                x: 15
                y: (parent.height - height) / 2
                width: 4
                height: parent.height - 40
                group: "[Master]"
                key: "VuMeterL"
            }

            Skin.VuMeter {
                x: parent.width - width - 15
                y: (parent.height - height) / 2
                width: 4
                height: parent.height - 40
                group: "[Master]"
                key: "VuMeterR"
            }

            Skin.ControlSlider {
                id: volumeSlider

                anchors.fill: parent
                group: "[Master]"
                key: "volume"
                barColor: Theme.volumeSliderBarColor
                bg: "images/slider_volume.svg"
            }

        }

        Skin.MixerColumn {
            width: 56
            height: parent.height
            group: "[Channel1]"
        }

        Skin.MixerColumn {
            width: 56
            height: parent.height
            group: "[Channel2]"
        }

        Skin.MixerColumn {
            width: 56
            height: parent.height
            group: "[Channel3]"
        }

        Skin.MixerColumn {
            width: 56
            height: parent.height
            group: "[Channel4]"
        }

        Skin.MixerColumn {
            width: 56
            height: parent.height
            group: "[Sampler1]"
        }

        Skin.MixerColumn {
            width: 56
            height: parent.height
            group: "[Sampler2]"
        }

        Skin.MixerColumn {
            width: 56
            height: parent.height
            group: "[Microphone]"
        }

        Skin.MixerColumn {
            width: 56
            height: parent.height
            group: "[Microphone2]"
        }

        Skin.MixerColumn {
            width: 56
            height: parent.height
            group: "[Auxiliary1]"
        }

        Skin.MixerColumn {
            width: 56
            height: parent.height
            group: "[Auxiliary2]"
        }

    }

}
