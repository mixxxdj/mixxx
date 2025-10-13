import Mixxx 1.0 as Mixxx
import QtQuick.Shapes
import QtQuick 2.12
import ".." as Skin
import "../Theme"

Item {
    id: root

    required property string group

    property color buttonColor: trackLoadedControl.value > 0 ? Theme.buttonActiveColor : Theme.buttonDisableColor

    Mixxx.ControlProxy {
        id: trackLoadedControl

        group: root.group
        key: "track_loaded"
    }

    Skin.ControlButton {
        id: reverseButton

        implicitWidth: 22
        implicitHeight: 22

        group: root.group
        key: "reverse"
        contentItem: Shape {
            anchors.fill: parent
            antialiasing: true
            layer.enabled: true
            layer.samples: 4
            ShapePath {
                fillColor: root.buttonColor
                strokeColor: 'transparent'
                startX: 5; startY: 11
                PathLine { x: 20; y: 4 }
                PathLine { x: 20; y: 18 }
                PathLine { x: 5; y: 11 }
            }
        }
        activeColor: Theme.deckActiveColor
    }
    Skin.Button {
        id: beatgridButton

        visible: root.width > 165

        implicitHeight: 22
        anchors.right: keylockButton.left
        anchors.rightMargin: 5

        text: "Beatgrid"
    }
    Skin.Button {
        id: keylockButton

        visible: root.width > 105

        implicitHeight: 22
        anchors.right: ejectButton.left
        anchors.rightMargin: 5

        text: "Keylock"
    }
    Skin.ControlButton {
        id: ejectButton

        anchors.right: parent.right

        implicitWidth: 22
        implicitHeight: 22

        group: root.group
        key: "eject"
        contentItem: Item {
            anchors.fill: parent
            Shape {
                anchors {
                    top: parent.top
                    topMargin: 5
                    horizontalCenter: parent.horizontalCenter
                }
                width: 15
                height: 10
                antialiasing: true
                layer.enabled: true
                layer.samples: 4
                ShapePath {
                    fillColor: root.buttonColor
                    strokeColor: 'transparent'
                    startX: 7.5; startY: 0
                    PathLine { x: 15; y: 10 }
                    PathLine { x: 0; y: 10 }
                    PathLine { x: 7.5; y: 0 }
                }
            }
            Rectangle {
                width: 15
                height: 2
                color: root.buttonColor
                anchors {
                    bottom: parent.bottom
                    bottomMargin: 3
                    horizontalCenter: parent.horizontalCenter
                }
            }
        }
        activeColor: Theme.deckActiveColor
    }
}
