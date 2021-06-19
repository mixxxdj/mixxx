import Mixxx 0.1 as Mixxx
import QtQuick 2.12
import QtQuick.Shapes 1.12

Item {
    id: root

    property string group // required
    property string key // required
    property string color: "white"

    Shape {
        id: shape

        visible: control.value >= 0
        anchors.fill: parent
        antialiasing: true
        layer.smooth: true
        layer.samples: 2

        ShapePath {
            startX: marker.x
            startY: 0
            strokeColor: root.color
            strokeWidth: 1

            PathLine {
                id: marker

                property bool hovered: false

                x: 0
                y: root.height
            }

        }

    }

    Mixxx.ControlProxy {
        id: control

        group: root.group
        key: root.key
        onValueChanged: marker.x = parent.width * value
    }

}
