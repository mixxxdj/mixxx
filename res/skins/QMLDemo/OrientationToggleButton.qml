import "." as Skin
import Mixxx 0.1 as Mixxx
import QtQuick 2.12
import QtQuick.Controls 2.12

AbstractButton {
    id: root

    property string group // required
    property string key // required
    property alias orientation: control.value
    property color color: "white"

    implicitWidth: 56
    implicitHeight: 26
    onClicked: control.value = Math.trunc(3 * pressX / root.width)
    states: [
        State {
            name: "left"
            when: orientation == 0

            PropertyChanges {
                target: indicator
                x: 0
            }

        },
        State {
            name: "right"
            when: orientation == 2

            PropertyChanges {
                target: indicator
                x: parent.width - width
            }

        },
        State {
            name: "mid"
            when: orientation == 1

            PropertyChanges {
                target: indicator
                x: parent.width / 2 - width / 2
            }

        }
    ]

    Mixxx.ControlProxy {
        id: control

        group: root.group
        key: root.key
    }

    Item {
        anchors.fill: root
        anchors.leftMargin: 10
        anchors.rightMargin: 10
        anchors.topMargin: 2
        anchors.bottomMargin: 2

        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            height: 2
            color: root.color
        }

        Rectangle {
            id: indicator

            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.margins: 5
            width: 3
            color: root.color

            Behavior on x {
                NumberAnimation {
                    duration: 150
                }

            }

        }

    }

}
