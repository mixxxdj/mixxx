import Mixxx 1.0 as Mixxx
import QtQuick 2.12
import QtQuick.Controls 2.12

Item {
    id: root

    required property string group
    property bool indicatorVisible: true
    property alias indicator: indicatorContainer.contentItem

    // Avoid animation short blinking of spinny during startup
    Component.onCompleted: indicatorTransition.enabled = true

    Mixxx.ControlProxy {
        id: samplesControl

        group: root.group
        key: "track_samples"
    }

    Mixxx.ControlProxy {
        id: sampleRateControl

        group: root.group
        key: "track_samplerate"
    }

    Mixxx.ControlProxy {
        id: playPositionControl

        group: root.group
        key: "playposition"
    }

    Mixxx.ControlProxy {
        id: vinylSpeedTypeControl

        group: root.group
        key: "vinylcontrol_speed_type"
    }

    Mixxx.ControlProxy {
        id: trackLoadedControl

        group: root.group
        key: "track_loaded"
    }

    Mixxx.ControlProxy {
        id: scratchEnableControl

        group: root.group
        key: "scratch_position_enable"
        value: scratchArea.pressed
    }

    Mixxx.ControlProxy {
        id: scratchPositionControl

        group: root.group
        key: "scratch_position"
    }

    // Use the deck's configured vinyl speed, falling back to 33.33 RPM
    readonly property real rpm: vinylSpeedTypeControl.value > 0
        ? vinylSpeedTypeControl.value : 33.33
    readonly property real rps: Math.PI * rpm / 60.0
    readonly property real frameRate: sampleRateControl.value

    Control {
        id: indicatorContainer

        anchors.fill: parent
        visible: opacity > 0

        contentItem: Rectangle {
            height: root.height / 2
            width: height / 12
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
        }

        transform: Rotation {
            id: indicatorRotation

            property real roundsPerSecond: root.rps / Math.PI
            property real totalFrames: samplesControl.value / 2
            property real positionSeconds: (!isNaN(sampleRateControl.value) && sampleRateControl.value > 0) ? playPositionControl.value * totalFrames / sampleRateControl.value : 0
            property real rotationFactor: indicatorRotation.roundsPerSecond * indicatorRotation.positionSeconds % 1

            origin.x: root.width / 2
            origin.y: root.height / 2
            angle: 360 * rotationFactor
        }

        states: State {
            name: "hidden"
            when: !root.indicatorVisible

            PropertyChanges {
                target: indicatorContainer
                opacity: 0
            }
        }

        transitions: Transition {
            id: indicatorTransition

            enabled: false
            to: "hidden"
            reversible: true

            PropertyAnimation {
                property: "opacity"
                duration: 150
            }
        }
    }

    MouseArea {
        id: scratchArea

        anchors.fill: parent
        enabled: trackLoadedControl.value > 0
        cursorShape: pressed ? Qt.ClosedHandCursor : Qt.OpenHandCursor

        property real lastAngle: 0

        function getAngle(x, y) {
            return Math.atan2(y - height / 2, x - width / 2);
        }

        onPressed: {
            scratchPositionControl.value = 0.0;
            lastAngle = getAngle(mouse.x, mouse.y);
        }

        onPositionChanged: {
            if (isNaN(sampleRateControl.value) || sampleRateControl.value <= 0) {
                console.error(`Could not find a valid sample rate on group ${root.group}, got ${sampleRateControl.value}`);
                return;
            }
            var currentAngle = getAngle(mouse.x, mouse.y);
            var delta = currentAngle - lastAngle;

            // Normalize to [-π, π] to handle atan2 boundary crossing
            while (delta > Math.PI) delta -= 2 * Math.PI;
            while (delta < -Math.PI) delta += 2 * Math.PI;

            lastAngle = currentAngle;

            // Convert angular delta (radians) to samples:
            // samples = radians * (frameRate * 2) / (2π * rps)
            //         = radians * frameRate / (π * rps)
            scratchPositionControl.value += delta * root.frameRate / root.rps;
        }
    }
}
