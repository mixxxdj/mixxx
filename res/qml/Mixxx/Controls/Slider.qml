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
            startX: root.horizontal ? barShape.width * (1 - root.bar.start) : (root.bar.axis >= 0 ? root.bar.axis - root.bar.margin : barShape.width * 0.5)
            startY: root.vertical ? barShape.height * (1 - root.bar.start) : (root.bar.axis >= 0 ? root.bar.axis - root.bar.margin : barShape.height * 0.5)
            strokeColor: root.bar.color
            strokeWidth: root.bar.width

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
