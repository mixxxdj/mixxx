import QtQuick
import QtQuick.Controls

SpinBox {
    id: control
    property color color: "#202020"
    implicitHeight: 24
    implicitWidth: 56
    leftPadding: 2
    rightPadding: 22
    topPadding: 2
    bottomPadding: 2
    font.bold: true

    background: Rectangle {
        color: "#000000"
        Rectangle {
            x: 1
            y: 1
            width: parent.width - 20
            height: parent.height - 2
            color: control.color
            radius: 2
            Rectangle {
                x: 1
                y: 1
                width: parent.width - 2
                height: parent.height - 2
                color: "#60000000"
                radius: 2
            }
            Rectangle {
                x: 1
                y: 1
                width: parent.width - 2
                height: 1
                color: "#000000"
            }
            Rectangle {
                x: 1
                y: 1
                width: 1
                height: parent.height - 2
                color: "#000000"
            }
        }
        Rectangle {
            x: parent.width - 20
            y: 0
            width: 1
            height: parent.height
            color: "#000000"
        }
        Rectangle {
            x: parent.width - 20
            y: 1
            width: 20 - 1
            height: parent.height - 2
            color: control.color
            radius: 2
            Rectangle {
                width: parent.width
                height: parent.height
                color: "transparent"
                border.color: control.pressed ? "#60000000" : "#04ffffff"
                border.width: 1
                radius: 2
            }
            Rectangle {
                width: parent.width
                height: 1
                color: control.pressed ? "#04ffffff" : "#0cffffff"
            }
        }
    }

    up.indicator: Rectangle {
        x: parent.width - 19
        y: 1
        width: 18
        height: 11
        radius: 2
        color: control.up.pressed ? "#257b82" : "transparent"
        Image {
            source: "assets/spinbox/indicator_up.svg"
        }
    }

    down.indicator: Rectangle {
        x: parent.width - 19
        y: parent.height / 2
        width: 18
        height: 11
        radius: 2
        color: control.down.pressed ? "#257b82" : "transparent"
        Image {
            source: "assets/spinbox/indicator_down.svg"
        }
    }

    contentItem: TextInput {
        text: control.textFromValue(control.value, control.locale)
        font: control.font
        color: "#908070"
        selectionColor: "#257b82"
        selectedTextColor: "#ffffff"
        horizontalAlignment: Qt.AlignHCenter
        verticalAlignment: Qt.AlignVCenter

        readOnly: !control.editable
        validator: control.validator
        inputMethodHints: Qt.ImhFormattedNumbersOnly

        onAccepted: control.focus = false

        Rectangle {
            x: 0
            y: 0
            visible: control.activeFocus
            width: parent.width
            height: parent.height
            color: "transparent"
            border.color: "#257b82"
            border.width: 1
        }
    }
}
