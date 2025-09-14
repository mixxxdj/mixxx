import ".." as Skin
import Mixxx 1.0 as Mixxx
import Mixxx.Controls 1.0 as MixxxControls
import QtQuick 2.12
import QtQuick.Controls 2.12
import "../Theme"

Item {
    id: root

    required property string group

    Rectangle {
        id: spinner
        radius: width / 2
        color: '#BDBDBD'
        transform: Rotation { origin.x: spinner.width/2; origin.y: spinner.height/2; angle: 45}

        height: 140
        width: 140

        anchors {
            top: parent.top
        }

        MixxxControls.Spinny {
            id: spinnyIndicator

            anchors.fill: parent
            group: root.group
            indicatorVisible: trackLoadedControl.value

            indicator: Item {
                width: spinnyIndicator.width
                height: spinnyIndicator.height

                Rectangle {
                    id: tick
                    width: 2
                    height: 70
                    color: '#0E0E0E'
                    anchors {
                        top: parent.top
                        horizontalCenter: parent.horizontalCenter
                    }
                }
            }
        }
    }
    Rectangle {
        id: tempoInfo
        anchors.centerIn: spinner
        width: spinner.width /2 - 10
        height: trackLoadedControl.value ? spinner.height / 4 : 0
        clip: true
        radius: height / 2 - 5
        color: '#0E0E0E'

        Behavior on height {
            SpringAnimation {
                id: heightAnimation
                duration: 500
                spring: 2
                damping: 0.2
            }
        }

        Label {
            id: tempo

            Mixxx.ControlProxy {
                id: bpmControl

                group: root.group
                key: "bpm"
            }

            readonly property real bpm: bpmControl.value
            readonly property int precision: 2

            color: '#BDBDBD'
            text: `${Math.round(bpm)}.${((bpm%1)*Math.pow(10, precision)).toFixed().padEnd(precision, "0")}`
            anchors {
                top: parent.top
                topMargin: 2
                horizontalCenter: parent.horizontalCenter
            }
        }
        Label {
            id: pitchRatio

            Mixxx.ControlProxy {
                id: rateRatioControl

                group: root.group
                key: "rate_ratio"
            }

            readonly property real ratio: ((rateRatioControl.value - 1) * 100).toPrecision(2)

            color: '#3F3F3F'
            text: ((ratio > 0) ? "+" + ratio.toFixed(2) : ratio.toFixed(2)) + "%"
            anchors {
                bottom: parent.bottom
                bottomMargin: 2
                horizontalCenter: parent.horizontalCenter
            }
        }
    }
}
