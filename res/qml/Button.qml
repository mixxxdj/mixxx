import Qt5Compat.GraphicalEffects
import QtQuick 2.12
import QtQuick.Controls 2.12
import "Theme"

AbstractButton {
    id: root

    required property color activeColor
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
            color: Theme.darkGray2
            radius: 0
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
            color: Theme.darkGray
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
                color: root.checked ? Theme.accentColor : Theme.darkGray3
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
                color: Theme.accentColor
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
            PropertyChanges {
                color: Qt.darker(Theme.accentColor, 3)
                target: bottomInnerEffect
            }
            PropertyChanges {
                color: Qt.darker(Theme.accentColor, 3)
                target: topInnerEffect
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
