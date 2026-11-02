import Mixxx 1.0 as Mixxx
import QtQuick
import "../Theme"

Rectangle {
    id: root

    property bool active: false

    signal pressed

    border.color: root.active ? Theme.accentColor : Theme.white
    border.width: 1
    color: 'transparent'
    radius: height / 2

    MouseArea {
        id: mouseArea

        anchors.fill: parent

        onPressed: root.pressed()
    }
    Text {
        anchors.centerIn: parent
        color: root.active ? Theme.accentColor : Theme.white
        text: "!"
    }
}
