import "." as Skin
import Mixxx 1.0 as Mixxx
import QtQuick 2.12

import "Theme"

Skin.Button {
    id: root

    required property string keyPrefix
    required property string group

    activeColor: Theme.deckActiveColor
    highlight: enabledControl.value

    Mixxx.ControlProxy {
        id: control

        group: root.group
        key: `${root.keyPrefix}_activate`
        value: root.down
    }

    Mixxx.ControlProxy {
        id: enabledControl

        group: root.group
        key: `${root.keyPrefix}_enabled`
    }

    Mixxx.ControlProxy {
        id: cleanControl

        group: root.group
        key: `${root.keyPrefix}_clear`
        value: mousearea.pressed && enabledControl.value
    }

    MouseArea {
        id: mousearea

        anchors.fill: parent
        acceptedButtons: Qt.RightButton
    }
}
