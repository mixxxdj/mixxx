import QtQuick
import QtQuick.Layouts
import Mixxx 1.0 as Mixxx

RowLayout {
    id: root
    Layout.alignment: Qt.AlignHCenter
    property color selectedColor: null
    spacing: 3
    Repeater {
        model: Mixxx.Config.getHotcueColorPalette()
        Item {
            required property color modelData
            width: 14
            height: 14

            Rectangle {
                color: modelData
                anchors.centerIn: parent
                radius: 3
                width: root.selectedColor == modelData ? 14 : 8
                height: root.selectedColor == modelData ? 14 : 8

                MouseArea {
                    anchors.fill: parent
                    onPressed: {
                        root.selectedColor = modelData
                    }
                }
            }
        }
    }
}
