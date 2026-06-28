import QtQuick
import "../../../qml" as Skin

// A LateNight-styled button wrapper around shared Mixxx.ControlProxy behavior.
LegacyIconButton {
    id: root

    required property string group
    required property string key
    property string rightClickKey: ""
    property string pressAndHoldKey: ""
    property string displayKey: ""
    property bool toggleable: false
    property bool activateOnClick: false
    property bool ignoreActivePresses: false
    property bool releaseToZero: true
    property bool longPressLatching: false
    property int numberStates: 2
    property int longPressDuration: 300
    property color longPressLatchOverlayColor: "transparent"
    property url longPressLatchOverlayBackgroundSource: ""
    property url longPressLatchOverlayIconSource: ""
    property real activeOpacity: 1.0
    property real inactiveOpacity: 0.72
    property color activeOverlayColor: "transparent"
    property real activeDisplayThreshold: 0
    property bool visualActiveState: false

    signal primaryPressed(real displayValue)
    signal secondaryPressed(real displayValue, real mouseX, real mouseY)

    readonly property bool isActive: controlBehavior.isActive
    readonly property bool isVisuallyActive: controlBehavior.isVisuallyActive
    readonly property bool latchRevealRunning: root.longPressLatching &&
            controlBehavior.longPressAnimationRunning
    readonly property url effectiveLatchOverlayBackgroundSource:
            root.longPressLatchOverlayBackgroundSource.toString().length > 0
            ? root.longPressLatchOverlayBackgroundSource
            : root.effectiveBackgroundSource
    readonly property url effectiveLatchOverlayIconSource:
            root.longPressLatchOverlayIconSource.toString().length > 0
            ? root.longPressLatchOverlayIconSource
            : root.effectiveIconSource

    activeState: root.isVisuallyActive
    pressedState: controlBehavior.pressed
    latchOverlayVisible: root.latchRevealRunning
    latchOverlayProgress: controlBehavior.longPressProgress
    latchOverlayColor: root.longPressLatchOverlayColor
    latchOverlayBackgroundSource: root.effectiveLatchOverlayBackgroundSource
    latchOverlayIconSource: root.effectiveLatchOverlayIconSource
    contentOpacity: root.isVisuallyActive ? root.activeOpacity : root.inactiveOpacity

    function triggerPrimaryAction() {
        controlBehavior.triggerPrimaryAction();
    }

    function triggerPressAndHoldAction() {
        controlBehavior.triggerPressAndHoldAction();
    }

    function nextState(value) {
        return controlBehavior.nextState(value);
    }

    function startLatchReveal() {
        controlBehavior.startLatchReveal();
    }

    Skin.ControlProxyButtonBehavior {
        id: controlBehavior

        anchors.fill: parent
        group: root.group
        key: root.key
        rightClickKey: root.rightClickKey
        pressAndHoldKey: root.pressAndHoldKey
        displayKey: root.displayKey
        toggleable: root.toggleable
        activateOnClick: root.activateOnClick
        ignoreActivePresses: root.ignoreActivePresses
        releaseToZero: root.releaseToZero
        longPressLatching: root.longPressLatching
        numberStates: root.numberStates
        longPressDuration: root.longPressDuration
        activeDisplayThreshold: root.activeDisplayThreshold
        visualActiveState: root.visualActiveState

        onPrimaryPressed: function(displayValue) {
            root.primaryPressed(displayValue);
        }

        onSecondaryPressed: function(displayValue, mouseX, mouseY) {
            root.secondaryPressed(displayValue, mouseX, mouseY);
        }
    }

    Rectangle {
        anchors.fill: parent
        color: root.activeOverlayColor
        opacity: root.isActive ? 0.15 : 0
        radius: 2
        visible: root.activeOverlayColor !== Qt.rgba(0, 0, 0, 0)

        Behavior on opacity {
            NumberAnimation {
                duration: 80
            }
        }
    }
}
