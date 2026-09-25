import Mixxx 1.0 as Mixxx
import QtQuick 2.12
import QtQuick.Shapes
import QtQuick.Controls 2.12
import ".." as Skin
import "../Theme"

Skin.ControlButton {
    id: root

    property bool minimized: false

    activeColor: Theme.deckActiveColor
    key: "play"
    toggleable: trackLoadedControl.value == 1

    contentItem: Item {
        id: content

        Shape {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            anchors.topMargin: 6
            antialiasing: true
            height: 32
            layer.enabled: true
            layer.samples: 4
            visible: !root.minimized
            width: 30

            ShapePath {
                capStyle: ShapePath.RoundCap
                fillColor: trackLoadedControl.value > 0 ? Theme.buttonActiveColor : Theme.buttonDisableColor
                fillRule: ShapePath.WindingFill
                startX: 5
                startY: 0
                strokeColor: 'transparent'

                PathLine {
                    x: 30
                    y: 16
                }
                PathLine {
                    x: 5
                    y: 32
                }
                PathLine {
                    x: 5
                    y: 0
                }
            }
        }
        Label {
            id: label

            anchors.bottom: parent.bottom
            anchors.bottomMargin: 4
            anchors.horizontalCenter: parent.horizontalCenter
            color: '#626262'
            font.bold: true
            font.capitalization: Font.AllUppercase
            font.family: Theme.fontFamily
            font.pixelSize: Theme.buttonFontPixelSize
            horizontalAlignment: Text.AlignHCenter
            text: "Play"
            verticalAlignment: Text.AlignVCenter

            states: [
                State {
                    when: root.minimized

                    AnchorChanges {
                        anchors.bottom: undefined
                        anchors.verticalCenter: content.verticalCenter
                        target: label
                    }
                }
            ]
        }
    }

    Mixxx.ControlProxy {
        id: trackLoadedControl

        group: root.group
        key: "track_loaded"
    }
}
