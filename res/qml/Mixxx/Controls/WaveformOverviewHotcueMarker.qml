import Mixxx 1.0 as Mixxx
import QtQuick 2.12
import QtQuick.Shapes 1.12
import QtQuick.Window 2.12

Item {
    id: root

    required property string group
    required property int hotcueNumber
    readonly property real devicePixelRatio: Screen.devicePixelRatio > 0 ? Screen.devicePixelRatio : 1
    readonly property real markerX: trackSamplesControl.value > 0
            ? Math.round(root.width * (positionControl.value / trackSamplesControl.value) * root.devicePixelRatio) / root.devicePixelRatio
            : 0

    Shape {
        id: shape

        visible: statusControl.value >= 0
        anchors.fill: parent
        antialiasing: true
        layer.smooth: true
        layer.samples: 2

        ShapePath {
            startX: marker.x
            startY: 0
            strokeColor: colorControl.hotcueColor
            strokeWidth: 1

            PathLine {
                id: marker

                x: root.markerX
                y: root.height
            }
        }
    }

    Mixxx.ControlProxy {
        id: trackSamplesControl

        group: root.group
        key: "track_samples"
    }

    Mixxx.ControlProxy {
        id: statusControl

        group: root.group
        key: "hotcue_" + root.hotcueNumber + "_status"
    }

    Mixxx.ControlProxy {
        id: positionControl

        group: root.group
        key: "hotcue_" + root.hotcueNumber + "_position"
    }

    Mixxx.ControlProxy {
        id: colorControl

        property color hotcueColor: updateColor()

        function updateColor() {
            hotcueColor = (this.value >= 0) ? "#" + this.value.toString(16) : "transparent";
        }

        group: root.group
        key: "hotcue_" + root.hotcueNumber + "_color"
        onValueChanged: this.updateColor()
    }
}
