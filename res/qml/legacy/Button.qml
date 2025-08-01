import QtQuick
import QtQuick.Controls
// import skin.mixxx.org // for ColorScheme singleton

Button {
    id: control
    property color color: "transparent"
    implicitHeight: 24
    implicitWidth: Math.max(implicitHeight, Math.max(implicitBackgroundWidth + leftInset + rightInset, implicitContentWidth + leftPadding + rightPadding))
    font.bold: true

    contentItem: Text {
        text: control.text
        font: control.font
        opacity: enabled ? 1.0 : 0.3
        color: checked ? "#000000" : "#908070"
        // color: checked ? "#000000" : ColorScheme.buttonText
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }

    background: Rectangle {
        color: control.color
        Rectangle {
            width: parent.width
            height: parent.height
            border.color: "#000000"
            border.width: 1
            color: "transparent"
        }
        Rectangle {
            width: parent.width
            height: parent.height
            border.color: "#000000"
            border.width: 1
            radius: 2
            color: "transparent"
        }
        Rectangle {
            x: 1
            y: 1
            width: parent.width - 2
            height: parent.height - 2
            color: "transparent"
            border.color: control.pressed ? "#60000000" : "#04ffffff"
            border.width: 1
            radius: 2
        }
        Rectangle {
            x: 2
            y: 1
            width: parent.width - 3
            height: 1
            color: control.pressed ? "#04ffffff" : "#0cffffff"
        }
        Image {
            width: control.width
            height: control.height
            source: control.icon.source
            fillMode: Image.PreserveAspectFit
        }
    }
}
