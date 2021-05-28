import Mixxx 0.1 as Mixxx
import QtQuick 2.15

Item {
    id: root

    required property string group
    property int rpm: 33
    property bool indicatorVisible: true
    property alias indicatorDelegate: indicatorContainer.data

    function update() {
        if (!indicatorContainer.visible)
            return ;

        let positionNormalized = playPositionControl.value;
        let totalFrames = samplesControl.value / 2;
        let sampleRate = sampleRateControl.value;
        if (isNaN(positionNormalized) || isNaN(totalFrames) || isNaN(sampleRate) || totalFrames <= 0 || sampleRate <= 0)
            return ;

        // Convert playpos to seconds.
        let t = positionNormalized * totalFrames / sampleRate;
        // Bad samplerate or number of track samples.
        if (isNaN(t))
            return ;

        indicatorRotation.angle = (360 * rpm / 60 * t) % 360;
    }

    // Avoid animation short blinking of spinny during startup
    Component.onCompleted: indicatorTransition.enabled = true

    Mixxx.ControlProxy {
        id: samplesControl

        group: root.group
        key: "track_samples"
        onValueChanged: root.update()
    }

    Mixxx.ControlProxy {
        id: sampleRateControl

        group: root.group
        key: "track_samplerate"
        onValueChanged: root.update()
    }

    Mixxx.ControlProxy {
        id: playPositionControl

        group: root.group
        key: "playposition"
        onValueChanged: root.update()
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

            origin.x: root.width / 2
            origin.y: root.height / 2
            angle: 0
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
