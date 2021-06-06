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

    // TODO: Implement support for horizontal sliders
    orientation: Qt.Vertical
    wheelEnabled: true

    Shape {
        anchors.fill: parent
        antialiasing: true
        visible: root.bar

        ShapePath {
            id: barPath

            strokeColor: "transparent"
            strokeWidth: 2
            fillColor: "transparent"
            startX: root.width / 2
            startY: root.height - root.barMargin - (root.height - 2 * root.barMargin) * barStart

            PathLine {
                x: root.width / 2
                y: root.height - root.barMargin - (root.height - 2 * root.barMargin) * root.value
            }

        }

    }

}
