import Mixxx 1.0 as Mixxx
import QtQuick 2.12
import QtQuick.Shapes 1.12
import QtQuick.Window 2.12

Item {
    id: root

    required property string group
    required property string key
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
        onValueChanged: (value) => {
            // Math.round saves tons of CPU by avoiding redrawing for fractional pixel positions.
            marker.x = Math.round(root.width * value * Screen.devicePixelRatio) / Screen.devicePixelRatio;
        }
    }
}
