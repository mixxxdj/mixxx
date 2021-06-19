import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Shapes 1.12

Slider {
    id: root

    property bool bar: false
    property real barMargin: 0
    property alias barColor: barPath.strokeColor
    property alias barWidth: barPath.strokeWidth
    property real barStart: 0

    orientation: Qt.Vertical
    wheelEnabled: true

    Shape {
        id: barShape

        anchors.fill: parent
        anchors.margins: root.barMargin
        antialiasing: true
        visible: root.bar

        ShapePath {
            id: barPath

            strokeColor: "transparent"
            strokeWidth: 2
            fillColor: "transparent"
            startX: barShape.width * (root.horizontal ? (1 - barStart) : 0.5)
            startY: barShape.height * (root.vertical ? (1 - barStart) : 0.5)

            PathLine {
                x: root.horizontal ? (barShape.width * root.value) : barPath.startX
                y: root.vertical ? (barShape.height * (1 - root.value)) : barPath.startY
            }

        }

    }

}
