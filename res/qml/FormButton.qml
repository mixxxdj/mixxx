import QtQuick 2.12
import QtQuick.Effects
import QtQuick.Controls 2.12
import "Theme"

AbstractButton {
    id: root

    property color activeColor: Theme.deckActiveColor
    property color backgroundColor: Theme.darkGray3
    property bool highlight: false
    property color normalColor: Theme.white
    property color pressedColor: activeColor

    implicitHeight: 20
    implicitWidth: 98

    background: Rectangle {
        id: backgroundImage

        anchors.fill: parent
        color: root.backgroundColor
        radius: 4
        border {
            color: '#1C1C1C'
            width: 1
        }
    }
    contentItem: Item {
        anchors.fill: parent

        MultiEffect {
            id: labelGlow

            anchors.fill: parent
            source: label
            shadowEnabled: true
            shadowColor: label.color
            shadowBlur: 0.05
            shadowHorizontalOffset: 0
            shadowVerticalOffset: 0
        }
        Label {
            id: label

            anchors.fill: parent
            color: root.normalColor
            font.bold: true
            font.capitalization: Font.AllUppercase
            font.family: Theme.fontFamily
            font.pixelSize: Theme.buttonFontPixelSize
            horizontalAlignment: Text.AlignHCenter
            text: root.text
            verticalAlignment: Text.AlignVCenter
            visible: root.text != null
        }
        Image {
            id: image

            anchors.centerIn: parent
            asynchronous: true
            fillMode: Image.PreserveAspectFit
            height: icon.height
            source: icon.source
            width: icon.width
            layer.enabled: icon.source != null
            layer.effect: MultiEffect {
                brightness: 1.0
                colorization: 1.0
                colorizationColor: root.normalColor
            }
        }
    }
    states: [
        State {
            name: "pressed"
            when: root.pressed

            PropertyChanges {
                backgroundImage.color: root.checked ? "#3a60be" : root.backgroundColor
            }
            PropertyChanges {
                label.color: root.pressedColor
            }
            PropertyChanges {
                labelGlow.visible: true
            }
        },
        State {
            name: "active"
            when: (root.highlight || root.checked) && !root.pressed

            PropertyChanges {
                backgroundImage.color: "#2D4EA1"
            }
            PropertyChanges {
                label.color: root.activeColor
            }
            PropertyChanges {
                labelGlow.visible: true
            }
        },
        State {
            name: "inactive"
            when: !root.checked && !root.highlight && !root.pressed

            PropertyChanges {
                label.color: root.normalColor
            }
            PropertyChanges {
                labelGlow.visible: false
            }
        }
    ]
}
