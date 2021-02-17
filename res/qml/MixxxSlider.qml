import QtQuick 2.12
import QtQuick.Controls 2.12
import Mixxx 1.0

Item {
    id: root
    property alias group: control.group
    property alias key: control.key

    Slider {
        id: slider
        orientation: Qt.Vertical
        value: control.parameter
        wheelEnabled: true
        width: parent.width
        height: parent.height

        Control {
            id: control
            parameter: slider.value
        }

        handle: Image {
            source: "../skins/LateNight/classic/sliders/knob_volume_deck.svg"
            width: 42
            height: 19
            x: slider.leftPadding + slider.availableWidth / 2 - width / 2
            y: slider.visualPosition * (slider.height - height)
        }

        background: Image {
            source: "../skins/LateNight/classic/sliders/slider_volume_deck.svg"
        }
    }
}
