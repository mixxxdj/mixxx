import "." as Skin
import Mixxx 0.1 as Mixxx
import QtGraphicalEffects 1.12
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
            when: root.checked && !root.pressed

            PropertyChanges {
                target: colorOverlay
                color: root.activeColor
            }

        },
        State {
            name: "inactive"
            when: !root.checked && !root.pressed

            PropertyChanges {
                target: colorOverlay
                color: root.normalColor
            }

        }
    ]

    Mixxx.ControlProxy {
        group: root.group
        key: root.key
        value: root.checked || root.down
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
