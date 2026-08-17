import QtQuick
import QtQuick.Controls

MenuSeparator {
    id: root

    property color bottomColor: "#333333"
    property color topColor: "#000000"

    implicitHeight: 6

    contentItem: Item {
        Rectangle {
            color: root.topColor
            height: 1
            width: parent.width - 8
            x: 4
            y: 2
        }
        Rectangle {
            color: root.bottomColor
            height: 1
            width: parent.width - 8
            x: 4
            y: 3
        }
    }
}
