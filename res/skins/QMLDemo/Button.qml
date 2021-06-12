import "." as Skin
import QtGraphicalEffects 1.12
import QtQml 2.12
import QtQuick 2.12
import QtQuick.Controls 2.12
import "Theme"

AbstractButton {
    id: root

    property color normalColor: Theme.buttonNormalColor
    property color activeColor // required
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
                source: Theme.imgButtonPressed
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
                source: Theme.imgButton
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

            PropertyChanges {
                target: backgroundImage
                source: Theme.imgButton
            }

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

    background: BorderImage {
        id: backgroundImage

        anchors.fill: parent
        horizontalTileMode: BorderImage.Stretch
        verticalTileMode: BorderImage.Stretch
        source: Theme.imgButton

        border {
            top: 10
            left: 10
            right: 10
            bottom: 10
        }

    }

    contentItem: Item {
        anchors.fill: parent

        Glow {
            id: labelGlow

            anchors.fill: parent
            radius: 5
            spread: 0.1
            samples: 1 + radius * 2
            color: label.color
            source: label
        }

        Label {
            id: label

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

    }

}
