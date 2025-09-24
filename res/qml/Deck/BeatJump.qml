import ".." as Skin
import Mixxx 1.0 as Mixxx
import Qt5Compat.GraphicalEffects
import QtQuick 2.12
import QtQuick.Shapes
import QtQuick.Controls 2.12
import "../Theme"

Rectangle {
    id: root

    required property string group

    property color buttonColor: trackLoadedControl.value > 0 ? Theme.buttonActiveColor : Theme.buttonDisableColor

    color: Theme.deckBeatjumpBackgroundColor

    Label {
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        text: "Beatjump"
        color: Theme.deckBeatjumpLabelColor
    }

    Mixxx.ControlProxy {
        id: trackLoadedControl

        group: root.group
        key: "track_loaded"
    }

    Skin.ControlButton {
        id: jumpBackButton
        anchors {
            top: parent.top
            topMargin: 22
            left: parent.left
            leftMargin: 6
        }

        implicitWidth: 50
        implicitHeight: 26

        group: root.group
        key: "beatjump_backward"
        contentItem: Item {
            anchors.fill: parent
            Shape {
                anchors.centerIn: parent
                width: 21
                height: 14
                antialiasing: true
                layer.enabled: true
                layer.samples: 4
                ShapePath {
                    fillColor: root.buttonColor
                    strokeColor: 'transparent'
                    startX: 0; startY: 1
                    PathLine { x: 1; y: 1 }
                    PathLine { x: 1; y: 7 }
                    PathLine { x: 10; y: 1 }
                    PathLine { x: 10; y: 7 }
                    PathLine { x: 20; y: 1 }
                    PathLine { x: 21; y: 1 }
                    PathLine { x: 21; y: 13 }
                    PathLine { x: 20; y: 13 }
                    PathLine { x: 10; y: 7 }
                    PathLine { x: 10; y: 13 }
                    PathLine { x: 1; y: 7 }
                    PathLine { x: 1; y: 13 }
                    PathLine { x: 0; y: 13 }
                    PathLine { x: 0; y: 1 }
                }
            }
        }
    }
    Skin.ControlButton {
        id: jumpForwardButton

        anchors {
            top: parent.top
            topMargin: 22
            right: parent.right
            rightMargin: 6
        }

        implicitWidth: 50
        implicitHeight: 26

        group: root.group
        key: "beatjump_forward"
        contentItem: Item {
            anchors.fill: parent
            Shape {
                anchors.centerIn: parent
                width: 21
                height: 14
                antialiasing: true
                layer.enabled: true
                layer.samples: 4
                ShapePath {
                    fillColor: root.buttonColor
                    strokeColor: 'transparent'
                    startX: 0; startY: 1
                    PathLine { x: 10; y: 7 }
                    PathLine { x: 10; y: 1 }
                    PathLine { x: 20; y: 7 }
                    PathLine { x: 20; y: 1 }
                    PathLine { x: 21; y: 1 }
                    PathLine { x: 21; y: 13 }
                    PathLine { x: 20; y: 13 }
                    PathLine { x: 20; y: 7 }
                    PathLine { x: 10; y: 13 }
                    PathLine { x: 10; y: 7 }
                    PathLine { x: 0; y: 13 }
                    PathLine { x: 0; y: 1 }
                }
            }
        }
    }

    Skin.ControlButton {
        id: jumpSizeHalfButton
        anchors {
            bottom: parent.bottom
            bottomMargin: 7
            left: parent.left
            leftMargin: 6
        }

        implicitWidth: 22
        implicitHeight: 28

        group: root.group
        key: "beatjump_size_halve"
        contentItem: Item {
            anchors.fill: parent
            Shape {
                anchors.centerIn: parent
                width: 12
                height: 10
                antialiasing: true
                layer.enabled: true
                layer.samples: 4
                ShapePath {
                    fillColor: root.buttonColor
                    strokeColor: 'transparent'
                    startX: 0; startY: 5
                    PathLine { x: 12; y: 0 }
                    PathLine { x: 12; y: 10 }
                    PathLine { x: 0; y: 5 }
                }
            }
        }
    }
    Item {
        anchors {
            bottom: parent.bottom
            bottomMargin: 7
            left: jumpSizeHalfButton.right
            leftMargin: 5
            right: jumpSizeDoubleButton.left
            rightMargin: 5
        }
        implicitHeight: 28

        Rectangle {
            id: backgroundImage

            anchors.fill: parent
            color: '#2B2B2B'
        }

        DropShadow {
            anchors.fill: backgroundImage
            horizontalOffset: 0
            verticalOffset: 0
            radius: 1.0
            color: "#80000000"
            source: backgroundImage
        }
        InnerShadow {
            anchors.fill: backgroundImage
            radius: 1
            samples: 16
            horizontalOffset: -0
            verticalOffset: 0
            color: "#353535"
            source: backgroundImage
        }
        Mixxx.ControlProxy {
            id: beatjumpSize

            group: root.group
            key: "beatjump_size"
        }
        TextInput {
            anchors.centerIn: backgroundImage
            text: beatjumpSize.value < 1 ? `1/${1/beatjumpSize.value}` : beatjumpSize.value
            color: root.buttonColor

            function update() {
                this.text = Qt.bindind(function() { return beatjumpSize.value < 1 ? `1/${1/beatjumpSize.value}` : beatjumpSize.value; })
            }

            onAccepted: {
                this.focus = false
                let [num, denum] = this.text.split("/")
                if (denum !== undefined) {
                    denum = parseInt(denum)
                    if (Number.isNaN(denum)) {
                        return update()
                    }
                } else {
                    denum = 1
                }
                num = parseInt(num)
                if (Number.isNaN(num)) {
                    return update()
                }
                beatjumpSize.value = num / denum
            }
        }
    }

    Skin.ControlButton {
        id: jumpSizeDoubleButton
        anchors {
            bottom: parent.bottom
            bottomMargin: 7
            right: parent.right
            rightMargin: 6
        }

        implicitWidth: 22
        implicitHeight: 28

        group: root.group
        key: "beatjump_size_double"
        contentItem: Item {
            anchors.fill: parent
            Shape {
                anchors.centerIn: parent
                width: 12
                height: 10
                antialiasing: true
                layer.enabled: true
                layer.samples: 4
                ShapePath {
                    fillColor: root.buttonColor
                    strokeColor: 'transparent'
                    startX: 0; startY: 0
                    fillRule: ShapePath.WindingFill
                    capStyle: ShapePath.RoundCap
                    PathLine { x: 12; y: 5 }
                    PathLine { x: 0; y: 10 }
                    PathLine { x: 0; y: 0 }
                }
            }
        }
    }
}
