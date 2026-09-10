import Mixxx 1.0 as Mixxx
import QtQuick 2.12

Item {
    id: root

    required property string group
    required property string key
    property string rightClickKey: ""
    property string pressAndHoldKey: ""
    property string displayKey: ""
    property bool toggleable: false
    property bool activateOnClick: false
    property bool ignoreActivePresses: false
    property bool powerWindow: false
    property bool releaseToZero: true
    property bool longPressLatching: false
    property bool handlePointerInput: true
    // Touchscreens have no right mouse button. Where the secondary action is
    // the only way to reach a feature (e.g. the hotcue popup), a long press
    // stands in for it. Only meaningful if pressAndHoldKey is unset, since
    // that already claims the long press.
    property bool longPressTriggersSecondary: false
    property int numberStates: 2
    property int longPressDuration: 300
    property real activeDisplayThreshold: 0
    property bool visualActiveState: false
    property bool pressAndHoldTriggered: false
    property bool longPressAnimationRunning: false
    property real longPressProgress: 0
    property real pressStartValue: 0
    property real pressTargetValue: 0
    readonly property real displayValue: displayControl.value
    readonly property bool isActive: displayControl.value > root.activeDisplayThreshold
    readonly property bool isVisuallyActive: root.isActive || root.visualActiveState
    readonly property bool pressed: primaryHandler.pressed || secondaryArea.pressed

    signal primaryPressed(real displayValue)
    signal secondaryPressed(real displayValue, real mouseX, real mouseY)

    function triggerPrimaryAction() {
        if (root.ignoreActivePresses && displayControl.value > root.activeDisplayThreshold) {
            return;
        }
        if (root.toggleable) {
            root.toggleControl();
        } else {
            control.value = 1;
        }
    }

    function toggleControl() {
        control.value = !control.value;
    }

    function triggerPressAndHoldAction() {
        holdControl.value = !holdControl.value;
    }

    function nextState(value) {
        return (Math.floor(value) + 1) % root.numberStates;
    }

    function startLatchReveal() {
        longPressTimer.stop();
        longPressProgressAnimation.stop();
        root.longPressProgress = 0;
        root.longPressAnimationRunning = true;
        longPressProgressAnimation.start();
        longPressTimer.start();
    }

    function pressPrimary() {
        root.pressAndHoldTriggered = false;
        root.primaryPressed(displayControl.value);
        if (root.powerWindow) {
            root.pressStartValue = displayControl.value;
            root.pressTargetValue = root.nextState(root.pressStartValue);
            powerWindowTimer.stop();
            control.value = root.pressTargetValue;
            powerWindowTimer.start();
        } else if (root.longPressLatching) {
            root.pressStartValue = displayControl.value;
            root.pressTargetValue = root.nextState(root.pressStartValue);
            longPressTimer.stop();
            longPressProgressAnimation.stop();
            root.longPressProgress = 0;
            control.value = root.pressTargetValue;
            if (root.pressStartValue <= root.activeDisplayThreshold &&
                    root.pressTargetValue > root.activeDisplayThreshold) {
                root.startLatchReveal();
            }
        } else if (!root.activateOnClick) {
            root.triggerPrimaryAction();
        }
    }

    function clickPrimary() {
        if (root.activateOnClick && !root.pressAndHoldTriggered) {
            root.triggerPrimaryAction();
        }
    }

    function releasePrimary() {
        if (root.powerWindow) {
            if (!powerWindowTimer.running) {
                control.value = 0;
            }
            powerWindowTimer.stop();
        } else if (root.longPressLatching) {
            if (longPressTimer.running &&
                    root.pressTargetValue > root.activeDisplayThreshold) {
                control.value = root.pressStartValue;
            }
            longPressTimer.stop();
            longPressProgressAnimation.stop();
            root.longPressAnimationRunning = false;
            root.longPressProgress = 0;
        } else if (!root.toggleable && !root.activateOnClick && root.releaseToZero) {
            control.value = 0;
        }
    }

    function pressSecondary(mouseX, mouseY) {
        root.secondaryPressed(displayControl.value, mouseX, mouseY);
        if (root.rightClickKey.length > 0) {
            rightControl.value = 1;
        }
    }

    function releaseSecondary() {
        if (root.rightClickKey.length > 0) {
            rightControl.value = 0;
        }
    }

    Mixxx.ControlProxy {
        id: control

        group: root.group
        key: root.key
    }

    Mixxx.ControlProxy {
        id: rightControl

        group: root.group
        key: root.rightClickKey.length > 0 ? root.rightClickKey : root.key
    }

    Mixxx.ControlProxy {
        id: holdControl

        group: root.group
        key: root.pressAndHoldKey.length > 0 ? root.pressAndHoldKey : root.key
    }

    Mixxx.ControlProxy {
        id: displayControl

        group: root.group
        key: root.displayKey.length > 0 ? root.displayKey : root.key
    }

    // A TapHandler (rather than a MouseArea) handles the primary action so
    // that touch points are tracked individually. With a MouseArea only the
    // one touch point that Qt synthesizes mouse events from is delivered,
    // which makes it impossible to press buttons on two decks at the same
    // time on a touchscreen.
    TapHandler {
        id: primaryHandler

        acceptedButtons: Qt.LeftButton
        enabled: root.enabled && root.handlePointerInput
        // Keep the grab while the finger moves, a tap is only emitted when
        // the release happens inside the button.
        gesturePolicy: TapHandler.ReleaseWithinBounds

        onPressedChanged: {
            if (primaryHandler.pressed) {
                root.pressPrimary();
            } else {
                root.releasePrimary();
            }
        }

        onLongPressed: {
            if (root.pressAndHoldKey.length > 0) {
                root.pressAndHoldTriggered = true;
                root.triggerPressAndHoldAction();
            } else if (root.longPressTriggersSecondary) {
                root.pressAndHoldTriggered = true;
                const position = primaryHandler.point.position;
                root.pressSecondary(position.x, position.y);
                root.releaseSecondary();
            }
        }

        onTapped: root.clickPrimary()
    }

    // The secondary action is mouse-only, so a plain MouseArea is used here.
    // Touch points never reach it because Qt synthesizes left button presses
    // from touch, which this MouseArea does not accept.
    MouseArea {
        id: secondaryArea

        acceptedButtons: Qt.RightButton
        anchors.fill: parent
        enabled: root.enabled && root.handlePointerInput

        onPressed: mouse => root.pressSecondary(mouse.x, mouse.y)
        onReleased: root.releaseSecondary()
    }

    Timer {
        id: longPressTimer

        interval: root.longPressDuration
        repeat: false
        onTriggered: {
            root.longPressAnimationRunning = false;
        }
    }

    Timer {
        id: powerWindowTimer

        interval: root.longPressDuration
        repeat: false
    }

    NumberAnimation {
        id: longPressProgressAnimation

        target: root
        property: "longPressProgress"
        from: 0
        to: 1
        duration: root.longPressDuration
        running: false
    }
}
