import ".." as Skin
import Mixxx 1.0 as Mixxx
import Mixxx.Controls 1.0 as MixxxControls
import Qt5Compat.GraphicalEffects
import QtQuick 2.12
import QtQuick.Layouts
import QtQuick.Shapes
import QtQuick.Controls 2.12
import "../Theme"

Item {
    id: root

    required property string group

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
                fillColor: '#D9D9D9'
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

        implicitHeight: 22
        anchors.right: keylockButton.left
        anchors.rightMargin: 5

        text: "Beatgrid"
    }
    Skin.Button {
        id: keylockButton

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
                    fillColor: '#D9D9D9'
                    startX: 7.5; startY: 0
                    PathLine { x: 15; y: 10 }
                    PathLine { x: 0; y: 10 }
                    PathLine { x: 7.5; y: 0 }
                }
            }
            Rectangle {
                width: 15
                height: 2
                color: '#D9D9D9'
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
