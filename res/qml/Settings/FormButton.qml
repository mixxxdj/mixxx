import QtQuick 2.12
import QtQuick.Effects
import QtQuick.Controls 2.12
import "../Theme"

AbstractButton {
    id: root

    property color activeColor: Theme.deckActiveColor
    property color backgroundColor: "#3F3F3F"
    property bool highlight: false
    property color normalColor: Theme.white
    property color pressedColor: activeColor

    implicitHeight: 20
    implicitWidth: 98

    background: Item {
        anchors.fill: parent

        Rectangle {
            id: backgroundImage

            anchors.fill: parent
            color: root.backgroundColor
            radius: 4
            border {
                color: 'transparent'
                width: 1
            }
        }
        Rectangle {
            anchors.fill: parent
            radius: 4
            gradient: Gradient {
                GradientStop { position: 0.0; color: Qt.rgba(0, 0, 0, 0.35) }
                GradientStop { position: 0.25; color: "transparent" }
                GradientStop { position: 0.75; color: "transparent" }
                GradientStop { position: 1.0; color: Qt.rgba(0, 0, 0, 0.12) }
            }
        }
        MultiEffect {
            anchors.fill: parent
            source: backgroundImage
            shadowEnabled: true
            shadowColor: "#0E0E0E"
            shadowBlur: 0.15
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
                backgroundImage.border.color: '#353535'
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
                backgroundImage.border.color: '#353535'
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
