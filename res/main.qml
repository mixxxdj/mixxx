import QtQuick 2.12
import QtQuick.Controls 2.12
import Mixxx 1.0

ApplicationWindow {
    id: window
    width: 34
    height: 100
    visible: true

    Slider {
        id: slider
        Control {
            id: volume
            group: "[Channel1]"
            key: "volume"
            parameter: slider.value
        }
        orientation: Qt.Vertical
        value: volume.parameter

        wheelEnabled: true

        width: window.width
        height: window.height
        handle: Image {
            source: "skins/LateNight/classic/sliders/knob_volume_deck.svg"
            width: 42
            height: 19
            x: slider.leftPadding + slider.availableWidth / 2 - width / 2
            y: slider.visualPosition * (slider.height - height)
        }
        background: Image {
            source: "skins/LateNight/classic/sliders/slider_volume_deck.svg"
        }
    }
}
