import Mixxx 0.1 as Mixxx
import QtGraphicalEffects 1.12
import QtQuick 2.12
import "Theme"

Rectangle {
    id: root

    property string group // required
    property string key // required
    property color barColor // required

    radius: width / 2
    color: "black"

    Mixxx.ControlProxy {
        id: control

        group: root.group
        key: root.key
    }

    Item {
        id: meterMask

        anchors.fill: parent
        visible: false

        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.margins: 1
            antialiasing: false // for performance reasons
            height: control.parameter * (parent.height - 2 * anchors.margins)
            radius: width / 2
        }

    }

    Rectangle {
        id: meterGradient

        antialiasing: false // for performance reasons
        anchors.fill: parent
        visible: false

        gradient: Gradient {
            GradientStop {
                position: 0.1
                color: Theme.red
            }

            GradientStop {
                position: 0.15
                color: Theme.yellow
            }

            GradientStop {
                position: 0.25
                color: Theme.yellow
            }

            GradientStop {
                position: 0.3
                color: Theme.green
            }

            GradientStop {
                position: 1
                color: Theme.green
            }

        }

    }

    OpacityMask {
        anchors.fill: parent
        source: meterGradient
        maskSource: meterMask
    }

}
