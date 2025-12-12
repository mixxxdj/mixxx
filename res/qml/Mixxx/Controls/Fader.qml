import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Shapes 1.12

Slider {
    id: root

    property bool bar: false
    property alias barColor: barPath.strokeColor
    property real barMargin: 0
    property real barStart: 0
    property alias barWidth: barPath.strokeWidth

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

            fillColor: "transparent"
            startX: barShape.width * (root.horizontal ? (1 - root.barStart) : 0.5)
            startY: barShape.height * (root.vertical ? (1 - root.barStart) : 0.5)
            strokeColor: "transparent"
            strokeWidth: 2

            PathLine {
                x: root.horizontal ? (barShape.width * root.value) : barPath.startX
                y: root.vertical ? (barShape.height * (1 - root.value)) : barPath.startY
            }
        }
    }
}
