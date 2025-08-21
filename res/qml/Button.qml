import Qt5Compat.GraphicalEffects
import QtQuick 2.12
import QtQuick.Controls 2.12
import "Theme"

AbstractButton {
    id: root

    property color normalColor: Theme.buttonNormalColor
    required property color activeColor
    property color pressedColor: activeColor
    property bool highlight: false

    implicitWidth: 52
    implicitHeight: 26
    states: [
        State {
            name: "pressed"
            when: root.pressed

            PropertyChanges {
                target: backgroundImage
                color: root.checked ? "#3a60be" : "#3F3F3F"
            }

            PropertyChanges {
                target: label
                color: root.pressedColor
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
                target: backgroundImage
                color: "#2D4EA1"
            }

            PropertyChanges {
                target: label
                color: root.activeColor
            }

            PropertyChanges {
                target: labelGlow
                visible: true
            }

        },
        State {
            name: "inactive"
            when: !root.checked && !root.highlight && !root.pressed

            // PropertyChanges {
            //     target: backgroundImage
            //     source: Theme.imgButton
            // }

            PropertyChanges {
                target: label
                color: root.normalColor
            }

            PropertyChanges {
                target: labelGlow
                visible: false
            }
        }
    ]

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
            horizontalOffset: 0
            verticalOffset: 0
            radius: 1.0
            color: "#80000000"
            source: backgroundImage
        }
        InnerShadow {
            id: effect2
            anchors.fill: backgroundImage
            radius: 1
            samples: 16
            horizontalOffset: 1
            verticalOffset: 1
            color: "#353535"
            source: effect1
        }
        InnerShadow {
            anchors.fill: backgroundImage
            radius: 1
            samples: 16
            horizontalOffset: -1
            verticalOffset: -1
            color: "#353535"
            source: effect2
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
