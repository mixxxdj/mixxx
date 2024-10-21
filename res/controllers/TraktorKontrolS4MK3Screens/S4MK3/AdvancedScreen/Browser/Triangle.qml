import QtQuick 2.15
import QtQuick.Shapes 1.7

Item {
    id: root

    property int borderWidth: 0
    property color color: "black"
    property color borderColor: "transparent"

    property alias antialiasing: triangle.antialiasing

    clip: false

    Shape {
        id: triangle
        anchors.centerIn: parent

        ShapePath {
            strokeWidth: root.borderWidth
            strokeColor: root.borderColor
            fillColor: root.color
            startX: 0; startY: 0
            PathLine { x: root.width; y: 0 }
            PathLine { x: 0.5* root.width; y: root.height }
            PathLine { x: 0; y: 0 }
        }
    }
}
