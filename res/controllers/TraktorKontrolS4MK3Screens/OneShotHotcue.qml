import QtQuick 2.15
import QtQuick.Shapes 1.4
import QtQuick.Window 2.15

import QtQuick.Controls 2.15

ShapePath {
    required property real position

    strokeWidth: 1
    strokeColor: Qt.rgba(0, 0, 0, 0.5)
    fillColor: "#f3dc6e"
    strokeStyle: ShapePath.SolidLine
    startX: position * Window.width; startY: 0

    PathLine { x: position * Window.width + 14; y: 0 }
    PathLine { x: position * Window.width + 21; y: 6 }
    PathLine { x: position * Window.width + 21; y: 7 }
    PathLine { x: position * Window.width + 14; y: 13 }
    PathLine { x: position * Window.width + 2; y: 13 }
    PathLine { x: position * Window.width + 2; y: 71 }
    PathLine { x: position * Window.width; y: 71 }
    PathLine { x: position * Window.width; y: 0 }
}
