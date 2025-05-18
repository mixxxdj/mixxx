import Mixxx 1.0 as Mixxx
import QtQuick
import "../Theme"

Rectangle {
    id: root

    property bool active: false

    color: 'transparent'
    border.color: root.active ? Theme.accentColor : Theme.white
    border.width: 1
    radius: height/2

    signal pressed

    MouseArea {
        id: mouseArea
        anchors.fill: parent

        onPressed: root.pressed()
    }
    Text {
        anchors.centerIn: parent
        text: "!"
        color: root.active ? Theme.accentColor : Theme.white
    }
}
