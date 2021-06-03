import Mixxx 0.1 as Mixxx
import QtQuick 2.12
import QtQuick.Shapes 1.12

Item {
    id: root

    property alias group: control.group
    property alias key: control.key
    property alias background: background.data
    property alias foreground: foreground.data
    property real min: 0
    property real max: 1
    property real angle: 130
    property bool arc: false
    property real arcRadius: width / 2
    property real arcOffsetX: 0
    property real arcOffsetY: 0
    property alias arcColor: arcPath.strokeColor
    property alias arcWidth: arcPath.strokeWidth
    property alias arcStyle: arcPath.strokeStyle
    property alias arcStylePattern: arcPath.dashPattern

    Mixxx.ControlProxy {
        id: control
    }

    Mixxx.ControlProxy {
        id: resetcontrol

        group: root.group
        key: root.key + "_set_default"
    }

    Item {
        id: background

        anchors.fill: parent
    }

    Item {
        id: foreground

        anchors.fill: parent
        rotation: (control.parameter - (root.max - root.min) / 2) * 2 * root.angle
    }

    Shape {
        anchors.fill: root
        antialiasing: true
        visible: root.arc

        ShapePath {
            id: arcPath

            strokeColor: "transparent"
            strokeWidth: 2
            fillColor: "transparent"

            PathAngleArc {
                startAngle: -90
                sweepAngle: (control.parameter - (root.max - root.min) / 2) * 2 * root.angle
                radiusX: root.arcRadius
                radiusY: root.arcRadius
                centerX: root.width / 2 + root.arcOffsetX
                centerY: root.width / 2 + root.arcOffsetY
            }

        }

    }

    MouseArea {
        id: mousearea

        property real posy: root.height / 2

        anchors.fill: root
        onWheel: {
            if (wheel.angleDelta.y < 0)
                control.parameter = Math.min(root.max, control.parameter + 0.1);
            else
                control.parameter = Math.max(root.min, control.parameter - 0.1);
        }
        onDoubleClicked: resetcontrol.value = 1
        onPressed: {
            mousearea.posy = mouse.y;
        }
        onPositionChanged: {
            if (mousearea.pressed) {
                const dy = mousearea.posy - mouse.y;
                let parameter = control.parameter + Math.max(Math.min(dy, 100), -100) / 100;
                control.parameter = Math.max(root.min, Math.min(root.max, parameter));
                mousearea.posy = mouse.y;
            }
        }
    }

}
