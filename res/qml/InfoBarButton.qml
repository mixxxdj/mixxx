import Mixxx 1.0 as Mixxx
import Qt5Compat.GraphicalEffects
import QtQuick 2.12
import QtQuick.Controls 2.12
import "Theme"

AbstractButton {
    id: root

    required property color activeColor
    property alias foreground: foreground.data
    required property string group
    property alias highlight: control.value
    required property string key
    property color normalColor: Theme.buttonNormalColor
    property color pressedColor: activeColor

    function toggle() {
        control.value = !control.value;
    }

    states: [
        State {
            name: "pressed"
            when: root.pressed

            PropertyChanges {
                color: root.pressedColor
                target: colorOverlay
            }
        },
        State {
            name: "active"
            when: root.highlight && !root.pressed

            PropertyChanges {
                color: root.activeColor
                target: colorOverlay
            }
        },
        State {
            name: "inactive"
            when: !root.highlight && !root.pressed

            PropertyChanges {
                color: root.normalColor
                target: colorOverlay
            }
        }
    ]

    onPressed: toggle()

    Mixxx.ControlProxy {
        id: control

        group: root.group
        key: root.key
    }
    Item {
        id: foreground

        anchors.fill: parent
    }
    ColorOverlay {
        id: colorOverlay

        anchors.fill: foreground
        color: root.normalColor
        source: foreground
    }
}
