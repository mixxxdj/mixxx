import QtQuick 2
import QtQuick.Effects
import QtQuick.Controls 2
import "Theme"

AbstractButton {
    id: root

    property color activeColor: Theme.buttonActiveColor
    property bool highlight: false
    property color normalColor: Theme.buttonNormalColor
    property color pressedColor: activeColor

    implicitHeight: 26
    implicitWidth: 52

    background: Rectangle {
        id: backgroundImage

        anchors.fill: parent
        color: '#2B2B2B'
        radius: 2
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
