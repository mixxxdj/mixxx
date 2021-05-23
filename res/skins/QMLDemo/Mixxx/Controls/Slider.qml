import Mixxx 0.1 as Mixxx
import QtQuick 2.12
import QtQuick.Controls 2.12

Item {
    id: root

    property alias group: control.group
    property alias key: control.key
    property alias handle: handleRect.data
    property alias background: slider.background

    Slider {
        id: slider

        width: root.width
        height: root.height
        orientation: Qt.Vertical
        wheelEnabled: true

        Mixxx.ControlProxy {
            id: control

            parameter: slider.value
            onValueChanged: slider.value = parameter
        }

        handle: Rectangle {
            id: handleRect

            x: slider.leftPadding + slider.availableWidth / 2 - childrenRect.width / 2
            y: slider.visualPosition * (slider.height - childrenRect.height)
        }

    }

}
