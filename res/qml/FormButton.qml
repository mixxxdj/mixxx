import Qt5Compat.GraphicalEffects
import QtQuick 2.12
import QtQuick.Controls 2.12
import "Theme"

AbstractButton {
    id: root

    property color normalColor: Theme.white
    property color backgroundColor: "#3F3F3F"
    property color activeColor: Theme.deckActiveColor
    property color pressedColor: activeColor
    property bool highlight: false

    implicitWidth: 98
    implicitHeight: 20
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

    background: Item {
        anchors.fill: parent

        Rectangle {
            id: backgroundImage
            visible: false

            anchors.fill: parent
            color: root.backgroundColor
            radius: 4
        }
        InnerShadow {
            id: bottomInnerEffect
            anchors.fill: parent
            radius: 8
            samples: 16
            spread: 0.3
            horizontalOffset: -1
            verticalOffset: -1
            color: "transparent"
            source: backgroundImage
        }
        InnerShadow {
            id: topInnerEffect
            anchors.fill: parent
            radius: 8
            samples: 16
            spread: 0.3
            horizontalOffset: 1
            verticalOffset: 1
            color: "transparent"
            source: bottomInnerEffect
        }

        DropShadow {
            id: dropEffect
            anchors.fill: parent
            horizontalOffset: 0
            verticalOffset: 0
            radius: 4.0
            color: "#0E0E0E"
            source: topInnerEffect
        }
    }

    contentItem: Item {
        anchors.fill: parent

        Glow {
            id: labelGlow

            anchors.fill: parent
            radius: 1
            spread: 0.1
            color: label.color
            source: label
        }

        Label {
            id: label

            visible: root.text != null

            anchors.fill: parent
            text: root.text
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            font.family: Theme.fontFamily
            font.capitalization: Font.AllUppercase
            font.bold: true
            font.pixelSize: Theme.buttonFontPixelSize
            color: root.normalColor
        }
        Image {
            id: image

            height: icon.height
            width: icon.width
            anchors.centerIn: parent

            source: icon.source
            fillMode: Image.PreserveAspectFit
            asynchronous: true
            visible: false
        }
        ColorOverlay {
            anchors.fill: image
            source: image
            visible: icon.source != null
            color: root.normalColor
            antialiasing: true
        }
    }
}
