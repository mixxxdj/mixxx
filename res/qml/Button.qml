import Qt5Compat.GraphicalEffects
import QtQuick 2.12
import QtQuick.Controls 2.12
import "Theme"

AbstractButton {
    id: root

    property color activeColor: Theme.buttonActiveColor
    property bool highlight: false
    property color normalColor: Theme.buttonNormalColor
    property color pressedColor: activeColor

    implicitHeight: 26
    implicitWidth: 52

    background: Item {
        anchors.fill: parent

        Rectangle {
            id: backgroundImage

            anchors.fill: parent
            color: '#2B2B2B'
            radius: 2
        }
        DropShadow {
            id: effect1

            anchors.fill: backgroundImage
            color: "#80000000"
            horizontalOffset: 0
            radius: 1.0
            source: backgroundImage
            verticalOffset: 0
        }
        InnerShadow {
            id: effect2

            anchors.fill: backgroundImage
            color: "#353535"
            horizontalOffset: 1
            radius: 1
            samples: 16
            source: effect1
            verticalOffset: 1
        }
        InnerShadow {
            anchors.fill: backgroundImage
            color: "#353535"
            horizontalOffset: -1
            radius: 1
            samples: 16
            source: effect2
            verticalOffset: -1
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
                color: root.checked ? "#3a60be" : Theme.darkGray3
                target: backgroundImage
            }
            PropertyChanges {
                color: root.pressedColor
                target: label
            }
            PropertyChanges {
                target: labelGlow
                visible: true
            }
        },
        State {
            name: "active"
            when: (root.highlight || root.checked) && !root.pressed

            PropertyChanges {
                color: "#2D4EA1"
                target: backgroundImage
            }
            PropertyChanges {
                color: root.activeColor
                target: label
            }
            PropertyChanges {
                target: labelGlow
                visible: true
            }
        },
        State {
            name: "inactive"
            when: !root.checked && !root.highlight && !root.pressed

            PropertyChanges {
                color: root.normalColor
                target: label
            }
            PropertyChanges {
                target: labelGlow
                visible: false
            }
        }
    ]
}
