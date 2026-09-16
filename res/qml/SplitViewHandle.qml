import QtQuick
import QtQuick.Controls 2.15
import QtQuick.Layouts
import "Theme"

Rectangle {
    id: root

    property int dotColumns: 1

    property color handleColor: SplitHandle.pressed || SplitHandle.hovered ? Theme.panelSplitterHandleActive : Theme.panelSplitterHandle
    property int handleSize: SplitHandle.pressed || SplitHandle.hovered ? 6 : 5

    clip: true
    color: Theme.panelSplitterBackground
    implicitHeight: 8
    implicitWidth: 8

    GridLayout {
        anchors.centerIn: parent
        columns: root.dotColumns

        Repeater {
            model: 3

            Rectangle {
                color: root.handleColor
                height: root.handleSize
                radius: root.handleSize
                width: root.handleSize
            }
        }
    }
}
