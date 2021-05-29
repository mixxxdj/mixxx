import Mixxx 0.1 as Mixxx
import QtQuick 2.12

Item {
    id: root

    required property string group
    property real rpm: 33
    property bool indicatorVisible: true
    property alias indicatorDelegate: indicatorContainer.data

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

    Item {
        id: indicatorContainer

        anchors.fill: parent
        visible: opacity > 0

        Item {
            anchors.fill: parent

            data: Rectangle {
                height: root.height / 2
                width: height / 12
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: parent.top
            }

        }

        transform: Rotation {
            id: indicatorRotation

            property real roundsPerSecond: root.rpm / 60
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

}
