import QtQuick
import QtQuick.Layouts
import Mixxx 1.0 as Mixxx

RowLayout {
    id: root

    property int currentIndex: 0
    property double padding: 10
    required property var palette

    spacing: 3

    // FIXME remove hardcoded width
    readonly property double cellSize: Math.min(20, Math.max(10, (240 - root.padding * 2) / root.palette.length))
    Repeater {
        model: root.palette
        Item {
            required property int index
            required property color modelData
            width: root.cellSize - root.spacing
            height: width

            Rectangle {
                color: modelData
                anchors.centerIn: parent
                radius: 3
                width: root.currentIndex == index ? parent.width : parent.width / 2
                height: root.currentIndex == index ? parent.height : parent.height / 2
            }
            MouseArea {
                anchors.fill: parent
                onPressed: {
                    root.currentIndex = index
                }
            }
        }
    }
}
