import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Shapes 1.12

Slider {
    id: root

    orientation: Qt.Vertical
    wheelEnabled: true

    Shape {
        id: barShape

        anchors.fill: parent
        anchors.margins: root.bar.margin
        antialiasing: true
        visible: root.bar.enabled

        ShapePath {
            id: barPath

            fillColor: "transparent"
            startX: barShape.width * (root.horizontal ? (1 - root.bar.start) : 0.5)
            startY: barShape.height * (root.vertical ? (1 - root.bar.start) : 0.5)
            strokeColor: "transparent"
            strokeWidth: 2

            PathLine {
                x: root.horizontal ? (barShape.width * root.value) : barPath.startX
                y: root.vertical ? (barShape.height * (1 - root.value)) : barPath.startY
            }
        }
    }

    component BarSettings: QtObject {
        property real axis: -1
        property color color: "transparent"
        property bool enabled: false
        property real margin: 0
        property real start: 0
        property real width: 2
    }
}
