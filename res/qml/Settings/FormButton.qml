import Qt5Compat.GraphicalEffects
import QtQuick 2.12
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
            visible: false
        }
        InnerShadow {
            id: bottomInnerEffect

            anchors.fill: parent
            color: "transparent"
            horizontalOffset: -1
            radius: 8
            samples: 16
            source: backgroundImage
            spread: 0.3
            verticalOffset: -1
        }
        InnerShadow {
            id: topInnerEffect

            anchors.fill: parent
            color: "transparent"
            horizontalOffset: 1
            radius: 8
            samples: 16
            source: bottomInnerEffect
            spread: 0.3
            verticalOffset: 1
        }
        DropShadow {
            id: dropEffect

            anchors.fill: parent
            color: "#0E0E0E"
            horizontalOffset: 0
            radius: 4.0
            source: topInnerEffect
            verticalOffset: 0
        }
    }
    contentItem: Item {
        anchors.fill: parent

        Glow {
            id: labelGlow

            anchors.fill: parent
            color: label.color
            radius: 1
            source: label
            spread: 0.1
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
            visible: false
            width: icon.width
        }
        ColorOverlay {
            anchors.fill: image
            antialiasing: true
            color: root.normalColor
            source: image
            visible: icon.source != null
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
                bottomInnerEffect.color: '#353535'
            }
            PropertyChanges {
                topInnerEffect.color: '#353535'
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
                bottomInnerEffect.color: '#353535'
            }
            PropertyChanges {
                topInnerEffect.color: '#353535'
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
