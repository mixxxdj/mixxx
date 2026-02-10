import Mixxx 1.0 as Mixxx
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

    property real angle: 130
    property bool arc: false
    property alias arcColor: arcPath.strokeColor
    property real arcOffsetX: 0
    property real arcOffsetY: 0
    property real arcRadius: width / 2
    property int arcStart: Knob.Center
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
    property alias arcStyle: arcPath.strokeStyle
    property alias arcStylePattern: arcPath.dashPattern
    property alias arcWidth: arcPath.strokeWidth
    property alias background: background.data
    property alias foreground: foreground.data
    property real max: 1
    property real min: 0
    property real value: min
    readonly property real valueCenter: (max - min) / 2
    property real wheelStepSize: (root.max - root.min) / 100

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
        // Enable smooth curves. For QtQuick Shapes, this currently only works
        // by enabling multisampling, so we use 4xMSAA here.
        // See https://www.qt.io/blog/2017/07/07/let-there-be-shapes for details.
        property int multiSamplingLevel: Mixxx.Config.multiSamplingLevel

        anchors.fill: parent
        antialiasing: true
        layer.enabled: multiSamplingLevel > 1
        layer.samples: multiSamplingLevel
        visible: root.arc

        ShapePath {
            id: arcPath

            fillColor: "transparent"
            strokeColor: "transparent"
            strokeWidth: 2

            PathAngleArc {
                centerX: root.width / 2 + root.arcOffsetX
                centerY: root.width / 2 + root.arcOffsetY
                radiusX: root.arcRadius
                radiusY: root.arcRadius
                startAngle: root.angleFrom(root.arcStartValue - root.valueCenter) - 90
                sweepAngle: root.angleFrom(root.value - root.arcStartValue)
            }
        }
    }
    DragHandler {
        id: dragHandler

        property vector2d lastTranslation: Qt.vector2d(0, 0)
        property real value

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
        property: "value"
        target: root
        value: dragHandler.value
        when: dragHandler.active
    }

    // TODO: Replace this with a WheelHandler once we switch to Qt >= 5.14.
    MouseArea {
        id: wheelHandler

        property real value

        acceptedButtons: Qt.NoButton
        anchors.fill: parent

        onWheel: {
            const value = (wheel.angleDelta.y < 0) ? Math.min(root.max, root.value + root.wheelStepSize) : Math.max(root.min, root.value - root.wheelStepSize);
            root.turned(value);
            dragHandler.value = value;
        }
    }
    Binding {
        property: "value"
        target: root
        value: wheelHandler.value
        when: wheelHandler.drag.active
    }
}
