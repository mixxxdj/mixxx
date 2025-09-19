import QtQuick
import QtQuick.Layouts
import Mixxx 1.0 as Mixxx

Item {
    id: root

    property double cellSize: 15
    property int currentIndex: 0
    property double padding: 10
    required property var palette
    property double selectFactor: 1.3 + ((root.cellSize - 8) / 15) * 0.7

    height: 20

    Component.onCompleted: {
        root.cellSize = Qt.binding(function () {
            return Math.min(15, Math.max(8, (root.width - root.padding * 2) / root.palette.length));
        });
    }

    RowLayout {
        anchors.fill: parent
        anchors.verticalCenter: parent.verticalCenter
        spacing: 0

        Repeater {
            model: root.palette

            Item {
                required property int index
                required property color modelData

                Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                height: width
                width: root.cellSize

                Rectangle {
                    anchors.centerIn: parent
                    color: modelData
                    height: root.currentIndex == index ? parent.height : parent.height / root.selectFactor
                    radius: 3
                    width: root.currentIndex == index ? parent.width : parent.width / root.selectFactor
                }
                MouseArea {
                    anchors.fill: parent

                    onPressed: {
                        root.currentIndex = index;
                    }
                }
            }
        }
    }
}
