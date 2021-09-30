import Mixxx 0.1 as Mixxx
import QtQuick 2.12
import QtQuick.Shapes 1.12

Item {
    id: root

    property string group // required
    property int hotcueNumber // required

    function updatePosition() {
        let totalSamples = trackSamplesControl.value;
        marker.x = (totalSamples > 0) ? root.width * (positionControl.value / totalSamples) : 0;
    }

    onWidthChanged: updatePosition()

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

                x: 0
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
        onValueChanged: updatePosition()
    }

    Mixxx.ControlProxy {
        id: colorControl

        property color hotcueColor: updateColor()

        function updateColor() {
            hotcueColor = (value >= 0) ? "#" + value.toString(16) : "transparent";
        }

        group: root.group
        key: "hotcue_" + root.hotcueNumber + "_color"
        onValueChanged: updateColor()
    }

}
