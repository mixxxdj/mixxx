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
    property bool releaseToZero: true
    property bool longPressLatching: false
    property bool handlePointerInput: true
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
    readonly property bool pressed: interactionArea.pressed

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
        if (root.longPressLatching) {
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
        if (root.longPressLatching) {
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

    MouseArea {
        id: interactionArea

        anchors.fill: parent
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        enabled: root.enabled && root.handlePointerInput

        onPressed: function(mouse) {
            if (mouse.button === Qt.LeftButton) {
                root.pressPrimary();
            } else if (mouse.button === Qt.RightButton) {
                root.pressSecondary(mouse.x, mouse.y);
            }
        }

        onPressAndHold: {
            if (root.pressAndHoldKey.length > 0) {
                root.pressAndHoldTriggered = true;
                root.triggerPressAndHoldAction();
            }
        }

        onClicked: function(mouse) {
            if (mouse.button === Qt.LeftButton) {
                root.clickPrimary();
            }
        }

        onReleased: function(mouse) {
            if (mouse.button === Qt.LeftButton) {
                root.releasePrimary();
            } else if (mouse.button === Qt.RightButton) {
                root.releaseSecondary();
            }
        }
    }

    Timer {
        id: longPressTimer

        interval: root.longPressDuration
        repeat: false
        onTriggered: {
            root.longPressAnimationRunning = false;
        }
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
