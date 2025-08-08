import ".." as Skin
import QtQuick 2.12
import QtQuick.Shapes
import QtQuick.Controls 2.12
import "../Theme"

Skin.ControlButton {
    id: root

    property bool minimized: false

    key: "play"
    toggleable: true
    contentItem: Item {
        id: content
        Shape {
            visible: !root.minimized
            anchors.top: parent.top
            anchors.topMargin: 6
            anchors.horizontalCenter: parent.horizontalCenter
            antialiasing: true
            layer.enabled: true
            layer.samples: 4
            width: 30
            height: 32
            ShapePath {
                fillColor: '#D9D9D9'
                startX: 5; startY: 0
                fillRule: ShapePath.WindingFill
                capStyle: ShapePath.RoundCap
                PathLine { x: 30; y: 16 }
                PathLine { x: 5; y: 32 }
                PathLine { x: 5; y: 0 }
            }
        }
        Label {
            id: label
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 4
            anchors.horizontalCenter: parent.horizontalCenter

            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            font.family: Theme.fontFamily
            font.capitalization: Font.AllUppercase
            font.bold: true
            font.pixelSize: Theme.buttonFontPixelSize
            text: "Play"
            color: '#626262'
            states: [
                State {
                    when: root.minimized
                    AnchorChanges { target: label; anchors.bottom: undefined; anchors.verticalCenter: content.verticalCenter}
                }
            ]
        }
    }
    activeColor: Theme.deckActiveColor
}
