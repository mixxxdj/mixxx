import Mixxx 0.1 as Mixxx
import QtQml 2.12
import QtQuick 2.12
import QtQuick.Shapes 1.12

Item {
    id: root

    enum ArcStart {
        Minimum,
        Center,
        Maximum
    }

    property real value: min
    property alias background: background.data
    property alias foreground: foreground.data
    property real min: 0
    property real max: 1
    property real wheelStepSize: (root.max - root.min) / 10
    property real angle: 130
    property bool arc: false
    property int arcStart: Knob.Center
    property real arcRadius: width / 2
    readonly property real arcStartValue: {
        switch (arcStart) {
        case Knob.ArcStart.Minimum:
            return min;
        case Knob.ArcStart.Maximum:
            return max;
        default:
            return valueCenter;
        }
    }
    property real arcOffsetX: 0
    property real arcOffsetY: 0
    property alias arcColor: arcPath.strokeColor
    property alias arcWidth: arcPath.strokeWidth
    property alias arcStyle: arcPath.strokeStyle
    property alias arcStylePattern: arcPath.dashPattern
    readonly property real valueCenter: (max - min) / 2

    signal turned(real value)

    function angleFrom(targetValue) {
        return targetValue * 2 * root.angle;
    }

    Item {
        id: background

        anchors.fill: parent
    }

    Item {
        id: foreground

        anchors.fill: parent
        rotation: root.angleFrom(root.value - root.valueCenter)
    }

    Shape {
        anchors.fill: parent
        antialiasing: true
        visible: root.arc

        ShapePath {
            id: arcPath

            strokeColor: "transparent"
            strokeWidth: 2
            fillColor: "transparent"

            PathAngleArc {
                startAngle: root.angleFrom(root.arcStartValue - root.valueCenter) - 90
                sweepAngle: root.angleFrom(root.value - root.arcStartValue)
                radiusX: root.arcRadius
                radiusY: root.arcRadius
                centerX: root.width / 2 + root.arcOffsetX
                centerY: root.width / 2 + root.arcOffsetY
            }

        }

    }

    DragHandler {
        id: dragHandler

        property real value
        property vector2d lastTranslation: Qt.vector2d(0, 0)

        target: null
        onActiveChanged: {
            dragHandler.value = root.value;
            lastTranslation = Qt.vector2d(0, 0);
        }
        onTranslationChanged: {
            const diff_x = (translation.x - lastTranslation.x);
            const diff_y = (translation.y - lastTranslation.y);
            this.lastTranslation = translation;
            const y_dominant = Math.abs(diff_y) > Math.abs(diff_x);
            let dist = Math.sqrt(diff_x * diff_x + diff_y * diff_y);
            // If y is dominant, then treat an increase in dy as negative (y is
            // pointed downward). Otherwise, if y is not dominant and x has
            // decreased, then treat it as negative.
            if ((y_dominant && diff_y > 0) || (!y_dominant && diff_x < 0))
                dist = -dist;

            // For legacy (MIDI) reasons this is tuned to 127.
            let value = root.value + dist / 127;
            // Clamp to [0.0, 1.0].
            value = Mixxx.MathUtils.clamp(value, 0, 1);
            root.turned(value);
            dragHandler.value = value;
        }
    }

    Binding {
        when: dragHandler.active
        target: root
        property: "value"
        value: dragHandler.value
    }

    // TODO: Replace this with a WheelHandler once we switch to Qt >= 5.14.
    MouseArea {
        id: wheelHandler

        property real value

        anchors.fill: parent
        acceptedButtons: Qt.NoButton
        onWheel: {
            const value = (wheel.angleDelta.y < 0) ? Math.min(root.max, root.value + root.wheelStepSize) : Math.max(root.min, root.value - root.wheelStepSize);
            root.turned(value);
            dragHandler.value = value;
        }
    }

    Binding {
        when: wheelHandler.drag.active
        target: root
        property: "value"
        value: wheelHandler.value
    }

}
