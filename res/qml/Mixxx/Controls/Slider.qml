import QtQuick 2.12
import QtQuick.Shapes 1.12

Item {
    id: root

    enum SnapMode {
        NoSnap,
        SnapAlways,
        SnapOnRelease
    }

    property bool bar: false
    property alias background: backgroundItem.data
    property real barMargin: 0
    property alias barColor: barPath.strokeColor
    property alias barWidth: barPath.strokeWidth
    property real barStart: 0
    property real bottomPadding: 0
    property real from: 0
    property alias handle: handleItem.data
    property real leftPadding: 0
    property bool live: true
    property int orientation: Qt.Vertical
    property real rightPadding: 0
    property int snapMode: Slider.NoSnap
    property real stepSize: 0
    property real to: 1
    property real topPadding: 0
    property real value: from
    property bool wheelEnabled: true
    property real wheelStepSize: (root.to - root.from) / 100

    readonly property real availableHeight: Math.max(0, height - topPadding - bottomPadding)
    readonly property real availableWidth: Math.max(0, width - leftPadding - rightPadding)
    readonly property bool horizontal: orientation === Qt.Horizontal
    readonly property real position: valueToPosition(displayValue)
    readonly property bool pressed: dragHandler.active
    readonly property bool vertical: orientation === Qt.Vertical
    readonly property real visualPosition: vertical ? 1 - position : position

    readonly property real displayValue: dragHandler.active ? dragHandler.value : value

    signal moved(real value)

    activeFocusOnTab: true
    implicitHeight: Math.max(backgroundItem.implicitHeight, handleItem.implicitHeight)
    implicitWidth: Math.max(backgroundItem.implicitWidth, handleItem.implicitWidth)

    function clamp(targetValue) {
        return Math.max(Math.min(root.from, root.to), Math.min(Math.max(root.from, root.to), targetValue));
    }

    function snapValue(targetValue, isFinal) {
        if (root.stepSize <= 0 || root.snapMode === Slider.NoSnap || (!isFinal && root.snapMode !== Slider.SnapAlways)) {
            return targetValue;
        }

        const stepSize = Math.abs(root.stepSize);
        return root.from + Math.round((targetValue - root.from) / stepSize) * stepSize;
    }

    function stepValue(direction) {
        const rangeDirection = root.to >= root.from ? 1 : -1;
        return root.clamp(root.value + direction * rangeDirection * Math.abs(root.stepSize || root.wheelStepSize));
    }

    function valueAt(targetPosition) {
        targetPosition = Math.max(0, Math.min(1, targetPosition));
        return snapValue(root.from + targetPosition * (root.to - root.from), false);
    }

    function valueToPosition(targetValue) {
        if (root.from === root.to) {
            return 0;
        }
        return Math.max(0, Math.min(1, (targetValue - root.from) / (root.to - root.from)));
    }

    function applyInteractiveValue(targetValue, isFinal) {
        dragHandler.value = clamp(snapValue(targetValue, isFinal));
        if (root.live || isFinal) {
            root.moved(dragHandler.value);
        }
    }

    Keys.onPressed: event => {
        if (event.key === Qt.Key_Left || event.key === Qt.Key_Down) {
            root.moved(stepValue(-1));
            event.accepted = true;
        } else if (event.key === Qt.Key_Right || event.key === Qt.Key_Up) {
            root.moved(stepValue(1));
            event.accepted = true;
        } else if (event.key === Qt.Key_Home) {
            root.moved(root.from);
            event.accepted = true;
        } else if (event.key === Qt.Key_End) {
            root.moved(root.to);
            event.accepted = true;
        }
    }

    Item {
        id: backgroundItem

        anchors.fill: parent
        z: 0
    }

    Item {
        id: handleItem

        anchors.fill: parent
        z: 2
    }

    Shape {
        id: barShape

        anchors.fill: parent
        anchors.margins: root.barMargin
        antialiasing: true
        visible: root.bar
        z: 1

        ShapePath {
            id: barPath

            strokeColor: "transparent"
            strokeWidth: 2
            fillColor: "transparent"
            startX: barShape.width * (root.horizontal ? (1 - root.barStart) : 0.5)
            startY: barShape.height * (root.vertical ? (1 - root.barStart) : 0.5)

            PathLine {
                x: root.horizontal ? (barShape.width * root.position) : barPath.startX
                y: root.vertical ? (barShape.height * (1 - root.position)) : barPath.startY
            }
        }
    }

    DragHandler {
        id: dragHandler

        property bool dragged: false
        property vector2d lastTranslation: Qt.vector2d(0, 0)
        property real value: root.value

        acceptedButtons: Qt.LeftButton
        enabled: root.enabled
        target: null
        dragThreshold: 0

        onActiveChanged: {
            if (active) {
                root.forceActiveFocus();
                value = root.value;
                dragged = false;
                lastTranslation = Qt.vector2d(0, 0);
            } else if (dragged && (!root.live || root.snapMode === Slider.SnapOnRelease)) {
                root.applyInteractiveValue(value, true);
            }
        }
        onTranslationChanged: {
            const diff = root.horizontal ? (translation.x - lastTranslation.x) : -(translation.y - lastTranslation.y);
            lastTranslation = translation;
            if (diff === 0) {
                return;
            }
            dragged = true;
            const length = Math.max(1, root.horizontal ? root.availableWidth : root.availableHeight);
            root.applyInteractiveValue(value + (diff / length) * (root.to - root.from), false);
        }
    }
    Binding on value {
        value: dragHandler.value
        when: dragHandler.active && root.live
    }

    MouseArea {
        acceptedButtons: Qt.NoButton
        anchors.fill: parent
        enabled: root.enabled && root.wheelEnabled

        onWheel: {
            root.moved(root.stepValue(wheel.angleDelta.y < 0 ? 1 : -1));
        }
    }
}
