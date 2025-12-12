import ".." as Skin
import Mixxx 1.0 as Mixxx
import Mixxx.Controls 1.0 as MixxxControls
import QtQuick 2.12
import QtQuick.Controls 2.12
import "../Theme"

Item {
    id: root

    required property string group

    Mixxx.ControlProxy {
        id: trackLoadedControl

        group: root.group
        key: "track_loaded"
    }
    Rectangle {
        id: spinner

        color: '#BDBDBD'
        height: 140
        radius: width / 2
        width: 140

        transform: Rotation {
            angle: 45
            origin.x: spinner.width / 2
            origin.y: spinner.height / 2
        }

        anchors {
            top: parent.top
        }
        MixxxControls.Spinny {
            id: spinnyIndicator

            anchors.fill: parent
            group: root.group
            indicatorVisible: trackLoadedControl.value

            indicator: Item {
                height: spinnyIndicator.height
                width: spinnyIndicator.width

                Rectangle {
                    id: tick

                    color: '#0E0E0E'
                    height: 70
                    width: 2

                    anchors {
                        horizontalCenter: parent.horizontalCenter
                        top: parent.top
                    }
                }
            }
        }
    }
    Rectangle {
        id: tempoInfo

        anchors.centerIn: spinner
        clip: true
        color: '#0E0E0E'
        height: trackLoadedControl.value ? spinner.height / 4 : 0
        radius: height / 2 - 5
        width: spinner.width / 2 - 10

        Behavior on height {
            SpringAnimation {
                id: heightAnimation

                damping: 0.2
                duration: 500
                spring: 2
            }
        }

        Label {
            id: tempo

            readonly property real bpm: bpmControl.value
            readonly property int precision: 2

            color: '#BDBDBD'
            text: `${Math.round(bpm)}.${((bpm % 1) * Math.pow(10, precision)).toFixed().padEnd(precision, "0")}`

            Mixxx.ControlProxy {
                id: bpmControl

                group: root.group
                key: "bpm"
            }
            anchors {
                horizontalCenter: parent.horizontalCenter
                top: parent.top
                topMargin: 2
            }
        }
        Label {
            id: pitchRatio

            readonly property real ratio: ((rateRatioControl.value - 1) * 100).toPrecision(2)

            color: Theme.darkGray3
            text: ((ratio > 0) ? "+" + ratio.toFixed(2) : ratio.toFixed(2)) + "%"

            Mixxx.ControlProxy {
                id: rateRatioControl

                group: root.group
                key: "rate_ratio"
            }
            anchors {
                bottom: parent.bottom
                bottomMargin: 2
                horizontalCenter: parent.horizontalCenter
            }
        }
    }
}
