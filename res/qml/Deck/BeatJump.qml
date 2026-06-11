import ".." as Skin
import Mixxx 1.0 as Mixxx
import Qt5Compat.GraphicalEffects
import QtQuick 2.12
import QtQuick.Shapes
import QtQuick.Controls 2.12
import "../Theme"

Rectangle {
    id: root

    property color buttonColor: trackLoadedControl.value > 0 ? Theme.buttonActiveColor : Theme.buttonDisableColor
    required property string group

    color: Theme.deckBeatjumpBackgroundColor

    Label {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        color: Theme.deckBeatjumpLabelColor
        text: "Beatjump"
    }
    Mixxx.ControlProxy {
        id: trackLoadedControl

        group: root.group
        key: "track_loaded"
    }
    Skin.ControlButton {
        id: jumpBackButton

        group: root.group
        implicitHeight: 26
        implicitWidth: 50
        key: "beatjump_backward"

        contentItem: Item {
            anchors.fill: parent

            Shape {
                anchors.centerIn: parent
                antialiasing: true
                height: 14
                layer.enabled: true
                layer.samples: 4
                width: 21

                ShapePath {
                    fillColor: root.buttonColor
                    startX: 0
                    startY: 1
                    strokeColor: 'transparent'

                    PathLine {
                        x: 1
                        y: 1
                    }
                    PathLine {
                        x: 1
                        y: 7
                    }
                    PathLine {
                        x: 10
                        y: 1
                    }
                    PathLine {
                        x: 10
                        y: 7
                    }
                    PathLine {
                        x: 20
                        y: 1
                    }
                    PathLine {
                        x: 21
                        y: 1
                    }
                    PathLine {
                        x: 21
                        y: 13
                    }
                    PathLine {
                        x: 20
                        y: 13
                    }
                    PathLine {
                        x: 10
                        y: 7
                    }
                    PathLine {
                        x: 10
                        y: 13
                    }
                    PathLine {
                        x: 1
                        y: 7
                    }
                    PathLine {
                        x: 1
                        y: 13
                    }
                    PathLine {
                        x: 0
                        y: 13
                    }
                    PathLine {
                        x: 0
                        y: 1
                    }
                }
            }
        }

        anchors {
            left: parent.left
            leftMargin: 6
            top: parent.top
            topMargin: 22
        }
    }
    Skin.ControlButton {
        id: jumpForwardButton

        group: root.group
        implicitHeight: 26
        implicitWidth: 50
        key: "beatjump_forward"

        contentItem: Item {
            anchors.fill: parent

            Shape {
                anchors.centerIn: parent
                antialiasing: true
                height: 14
                layer.enabled: true
                layer.samples: 4
                width: 21

                ShapePath {
                    fillColor: root.buttonColor
                    startX: 0
                    startY: 1
                    strokeColor: 'transparent'

                    PathLine {
                        x: 10
                        y: 7
                    }
                    PathLine {
                        x: 10
                        y: 1
                    }
                    PathLine {
                        x: 20
                        y: 7
                    }
                    PathLine {
                        x: 20
                        y: 1
                    }
                    PathLine {
                        x: 21
                        y: 1
                    }
                    PathLine {
                        x: 21
                        y: 13
                    }
                    PathLine {
                        x: 20
                        y: 13
                    }
                    PathLine {
                        x: 20
                        y: 7
                    }
                    PathLine {
                        x: 10
                        y: 13
                    }
                    PathLine {
                        x: 10
                        y: 7
                    }
                    PathLine {
                        x: 0
                        y: 13
                    }
                    PathLine {
                        x: 0
                        y: 1
                    }
                }
            }
        }

        anchors {
            right: parent.right
            rightMargin: 6
            top: parent.top
            topMargin: 22
        }
    }
    Skin.ControlButton {
        id: jumpSizeHalfButton

        group: root.group
        implicitHeight: 28
        implicitWidth: 22
        key: "beatjump_size_halve"

        contentItem: Item {
            anchors.fill: parent

            Shape {
                anchors.centerIn: parent
                antialiasing: true
                height: 10
                layer.enabled: true
                layer.samples: 4
                width: 12

                ShapePath {
                    fillColor: root.buttonColor
                    startX: 0
                    startY: 5
                    strokeColor: 'transparent'

                    PathLine {
                        x: 12
                        y: 0
                    }
                    PathLine {
                        x: 12
                        y: 10
                    }
                    PathLine {
                        x: 0
                        y: 5
                    }
                }
            }
        }

        anchors {
            bottom: parent.bottom
            bottomMargin: 7
            left: parent.left
            leftMargin: 6
        }
    }
    Item {
        implicitHeight: 28

        anchors {
            bottom: parent.bottom
            bottomMargin: 7
            left: jumpSizeHalfButton.right
            leftMargin: 5
            right: jumpSizeDoubleButton.left
            rightMargin: 5
        }
        Rectangle {
            id: backgroundImage

            anchors.fill: parent
            color: '#2B2B2B'
        }
        DropShadow {
            anchors.fill: backgroundImage
            color: "#80000000"
            horizontalOffset: 0
            radius: 1.0
            source: backgroundImage
            verticalOffset: 0
        }
        InnerShadow {
            anchors.fill: backgroundImage
            color: "#353535"
            horizontalOffset: -0
            radius: 1
            samples: 16
            source: backgroundImage
            verticalOffset: 0
        }
        Mixxx.ControlProxy {
            id: beatjumpSize

            group: root.group
            key: "beatjump_size"
        }
        TextInput {
            function update() {
                this.text = Qt.binding(function () {
                    return beatjumpSize.value < 1 ? `1/${1 / beatjumpSize.value}` : beatjumpSize.value;
                });
            }

            anchors.centerIn: backgroundImage
            color: root.buttonColor
            text: beatjumpSize.value < 1 ? `1/${1 / beatjumpSize.value}` : beatjumpSize.value

            onAccepted: {
                this.focus = false;
                let [numerator, denominator] = this.text.split("/");
                if (denominator !== undefined) {
                    denominator = parseInt(denominator);
                    if (Number.isNaN(denominator)) {
                        return update();
                    }
                } else {
                    denominator = 1;
                }
                numerator = parseInt(numerator);
                if (Number.isNaN(numerator)) {
                    return update();
                }
                beatjumpSize.value = numerator / denominator;
            }
        }
    }
    Skin.ControlButton {
        id: jumpSizeDoubleButton

        group: root.group
        implicitHeight: 28
        implicitWidth: 22
        key: "beatjump_size_double"

        contentItem: Item {
            anchors.fill: parent

            Shape {
                anchors.centerIn: parent
                antialiasing: true
                height: 10
                layer.enabled: true
                layer.samples: 4
                width: 12

                ShapePath {
                    capStyle: ShapePath.RoundCap
                    fillColor: root.buttonColor
                    fillRule: ShapePath.WindingFill
                    startX: 0
                    startY: 0
                    strokeColor: 'transparent'

                    PathLine {
                        x: 12
                        y: 5
                    }
                    PathLine {
                        x: 0
                        y: 10
                    }
                    PathLine {
                        x: 0
                        y: 0
                    }
                }
            }
        }

        anchors {
            bottom: parent.bottom
            bottomMargin: 7
            right: parent.right
            rightMargin: 6
        }
    }
}
