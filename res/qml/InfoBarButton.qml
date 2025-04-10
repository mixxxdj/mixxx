import Mixxx 1.0 as Mixxx
import Qt5Compat.GraphicalEffects
import QtQuick 2.12
import QtQuick.Controls 2.12
import "Theme"

AbstractButton {
    id: root

    required property string group
    required property string key
    property alias foreground: foreground.data
    property color normalColor: Theme.buttonNormalColor
    required property color activeColor
    property color pressedColor: activeColor
    property alias highlight: control.value

    function toggle() {
        control.value = !control.value;
    }

    onPressed: toggle()
    states: [
        State {
            name: "pressed"
            when: root.pressed

            PropertyChanges {
                target: colorOverlay
                color: root.pressedColor
            }

        },
        State {
            name: "active"
            when: root.highlight && !root.pressed

            PropertyChanges {
                target: colorOverlay
                color: root.activeColor
            }

        },
        State {
            name: "inactive"
            when: !root.highlight && !root.pressed

            PropertyChanges {
                target: colorOverlay
                color: root.normalColor
            }
        }
    ]

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
        source: foreground
        color: root.normalColor
    }
}
