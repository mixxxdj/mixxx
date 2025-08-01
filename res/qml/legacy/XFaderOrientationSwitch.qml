import QtQuick
import QtQuick.Controls

Control {
    id: control
    property int value: 0
    padding: 0
    background: Rectangle {
        color: "#000000"
        radius: control.height / 2
    }
    contentItem: Item {
        implicitWidth: 32
        implicitHeight: 8
        Image {
            x: value * (contentItem.width - contentItem.height) / 2
            width: control.height
            height: control.height
            source: "image://svgmodifier/xfader_orientation/handle.svg"
        }
    }
    Binding on value  {
        when: mouse.pressed
        value: Math.max(0, Math.min(2, 2 * mouse.mouseX / control.width))
        restoreMode: Binding.RestoreBinding
    }
    MouseArea {
        id: mouse
        width: control.width
        height: control.height
    }
}
