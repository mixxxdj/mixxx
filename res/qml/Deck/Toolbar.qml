import Mixxx 1.0 as Mixxx
import QtQuick.Shapes
import QtQuick 2.12
import ".." as Skin
import "../Theme"

Item {
    id: root

    property color buttonColor: trackLoadedControl.value > 0 ? Theme.buttonActiveColor : Theme.buttonDisableColor
    required property string group

    Mixxx.ControlProxy {
        id: trackLoadedControl

        group: root.group
        key: "track_loaded"
    }
    Skin.ControlButton {
        id: reverseButton

        activeColor: Theme.deckActiveColor
        group: root.group
        implicitHeight: 22
        implicitWidth: 22
        key: "reverse"

        contentItem: Shape {
            anchors.fill: parent
            antialiasing: true
            layer.enabled: true
            layer.samples: 4

            ShapePath {
                fillColor: root.buttonColor
                startX: 5
                startY: 11
                strokeColor: 'transparent'

                PathLine {
                    x: 20
                    y: 4
                }
                PathLine {
                    x: 20
                    y: 18
                }
                PathLine {
                    x: 5
                    y: 11
                }
            }
        }
    }
    Skin.Button {
        id: beatgridButton

        anchors.right: ejectButton.left
        anchors.rightMargin: 5
        implicitHeight: 22
        text: "Beatgrid"
        visible: root.width > 165
    }
    Skin.ControlButton {
        id: ejectButton

        activeColor: Theme.deckActiveColor
        anchors.right: parent.right
        group: root.group
        implicitHeight: 22
        implicitWidth: 22
        key: "eject"

        contentItem: Item {
            anchors.fill: parent

            Shape {
                antialiasing: true
                height: 10
                layer.enabled: true
                layer.samples: 4
                width: 15

                anchors {
                    horizontalCenter: parent.horizontalCenter
                    top: parent.top
                    topMargin: 5
                }
                ShapePath {
                    fillColor: root.buttonColor
                    startX: 7.5
                    startY: 0
                    strokeColor: 'transparent'

                    PathLine {
                        x: 15
                        y: 10
                    }
                    PathLine {
                        x: 0
                        y: 10
                    }
                    PathLine {
                        x: 7.5
                        y: 0
                    }
                }
            }
            Rectangle {
                color: root.buttonColor
                height: 2
                width: 15

                anchors {
                    bottom: parent.bottom
                    bottomMargin: 3
                    horizontalCenter: parent.horizontalCenter
                }
            }
        }
    }
}
