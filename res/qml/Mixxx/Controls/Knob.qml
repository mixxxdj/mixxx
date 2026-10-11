import Mixxx 1.0 as Mixxx
import QtQml 2.12
import QtQuick 2.12
import QtQuick.Shapes 1.12
import QtQuick.Window 2.12

Item {
    id: root

    enum ArcStart {
        Minimum,
        Center,
        Maximum
    }

    property bool inputEnabled: true
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
    property real knobCenterOffsetX: 0
    property real knobCenterOffsetY: 0
    property real max: 1
    property real min: 0
    property real value: min
    readonly property real valueCenter: (max - min) / 2
    // QWheelEvent::angleDelta() reports one standard mouse-wheel step as 120.
    readonly property int wheelDeltaStep: 120
    property real wheelStepSize: (root.max - root.min) / root.wheelSteps
    // Match the legacy MIDI-style knob resolution across the control range.
    readonly property int wheelSteps: 127

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

        transform: Rotation {
            angle: root.angleFrom(root.value - root.valueCenter)
            origin.x: root.width / 2 + root.knobCenterOffsetX
            origin.y: root.height / 2 + root.knobCenterOffsetY
        }
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
                centerY: root.height / 2 + root.arcOffsetY
                radiusX: root.arcRadius
                radiusY: root.arcRadius
                startAngle: root.angleFrom(root.arcStartValue - root.valueCenter) - 90
                sweepAngle: root.angleFrom(root.value - root.arcStartValue)
            }
        }
    }
    MouseArea {
        id: input

        property point previousPosition

        anchors.fill: parent
        enabled: root.inputEnabled && (!root.Window.window || root.Window.window.active)
        acceptedButtons: Qt.LeftButton | Qt.MiddleButton
        hoverEnabled: true
        preventStealing: true
        cursorShape: pressed || wheelCursorTimer.running ? Qt.BlankCursor : Qt.ArrowCursor

        onPressed: mouse => {
            previousPosition = Qt.point(mouse.x, mouse.y);
            wheelCursorTimer.stop();
        }
        onPositionChanged: mouse => {
            if (!pressed) {
                return;
            }
            const dx = mouse.x - previousPosition.x;
            const dy = mouse.y - previousPosition.y;
            previousPosition = Qt.point(mouse.x, mouse.y);
            let distance = Math.sqrt(dx * dx + dy * dy);
            if (Math.abs(dy) > Math.abs(dx) ? dy > 0 : dx < 0) {
                distance = -distance;
            }
            root.turned(Mixxx.MathUtils.clamp(root.value + distance / root.wheelSteps,
                    root.min, root.max));
        }
        onWheel: event => {
            const adjustment = event.angleDelta.y / root.wheelDeltaStep * root.wheelStepSize;
            root.turned(Mixxx.MathUtils.clamp(root.value + adjustment, root.min, root.max));
            wheelCursorTimer.restart();
            event.accepted = true;
        }
        onExited: wheelCursorTimer.stop()
    }
    Timer {
        id: wheelCursorTimer

        interval: 800
    }
    MouseArea {
        parent: root.Window.window ? root.Window.window.contentItem : null
        anchors.fill: parent
        z: 1000000
        visible: input.pressed
        acceptedButtons: Qt.NoButton
        hoverEnabled: true
        cursorShape: Qt.BlankCursor

        onWheel: event => event.accepted = false
    }
}
