import QtQuick
import QtQuick.Controls

Frame {
    id: control
    property color color: "transparent"
    background: Rectangle {
        color: control.color
        Rectangle {
            width: parent.width
            height: 1
            color: "#04ffffff"
        }
        Rectangle {
            width: 1
            height: parent.height
            color: "#04ffffff"
        }
        Rectangle {
            y: parent.height - 1
            width: parent.width
            height: 1
            color: "#80000000"
        }
        Rectangle {
            x: parent.width - 1
            width: 1
            height: parent.height
            color: "#80000000"
        }
    }
}
