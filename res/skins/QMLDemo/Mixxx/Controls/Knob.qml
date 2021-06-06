import Mixxx 0.1 as Mixxx
import QtQuick 2.12
import QtQuick.Shapes 1.12

Item {
    id: root

    property real value: min
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

    signal turned(real value)

    Item {
        id: background

        anchors.fill: parent
    }

    Item {
        id: foreground

        anchors.fill: parent
        rotation: (root.value - (root.max - root.min) / 2) * 2 * root.angle
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
                sweepAngle: (root.value - (root.max - root.min) / 2) * 2 * root.angle
                radiusX: root.arcRadius
                radiusY: root.arcRadius
                centerX: root.width / 2 + root.arcOffsetX
                centerY: root.width / 2 + root.arcOffsetY
            }

        }

    }

    DragHandler {
        id: dragHandler

        property real valueRange: root.max - root.min
        property vector2d lastTranslation: Qt.vector2d(0, 0)

        target: null
        grabPermissions: PointerHandler.CanTakeOverFromAnything | PointerHandler.ApprovesTakeOverByAnything
        onActiveChanged: lastTranslation = Qt.vector2d(0, 0)
        onTranslationChanged: {
            const delta = lastTranslation.y - translation.y;
            const change = valueRange * Mixxx.MathUtils.clamp(delta, -100, 100) / 100;
            const value = Mixxx.MathUtils.clamp(root.value + change, root.min, root.max);
            lastTranslation = translation;
            root.turned(value);
            root.value = value;
        }
    }

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.NoButton
        onWheel: {
            const value = (wheel.angleDelta.y < 0) ? Math.min(root.max, root.value + 0.1) : Math.max(root.min, root.value - 0.1);
            root.turned(value);
            root.value = value;
        }
    }

}
