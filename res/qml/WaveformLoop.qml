import Mixxx 1.0 as Mixxx
import QtQuick 2.15
import QtQuick.Shapes 1.4
import QtQuick.Window 2.15

import QtQuick.Controls 2.15

import "Theme"

Item {
    id: root

    property color mainColor: Theme.waveformMarkerLoopColor
    property color disabledColor: Theme.waveformMarkerLoopColorDisabled
    property real enabledOpacity: 0.8
    property real disabledOpacity: 0.5

    property real markerHeight: root.height
    property real radiusSize: 4
    property bool enabled: true

    Rectangle {
        anchors.fill: parent
        color: Qt.alpha(root.enabled ? root.mainColor : root.disabledColor, root.enabled ? root.enabledOpacity : root.disabledOpacity)
    }

    Shape {
        ShapePath {
            strokeWidth: 0
            strokeColor: 'transparent'
            fillColor: root.mainColor
            strokeStyle: ShapePath.SolidLine
            startX: -1; startY: 0

            PathLine { x: -16 + radiusSize; y: 0 }
            PathArc {
                x: -16
                y: radiusSize
                radiusX: radiusSize; radiusY: radiusSize
                direction: PathArc.Counterclockwise
            }
            PathLine { x: -16; y: 16 - radiusSize }
            PathArc {
                x: -16 + radiusSize
                y: 16
                radiusX: radiusSize; radiusY: radiusSize
                direction: PathArc.Counterclockwise
            }
            PathLine { x: -1; y: 16 }
            PathLine { x: -1; y: markerHeight }
            PathLine { x: 1; y: markerHeight }
            PathLine { x: 1; y: 0 }
        }
    }
    Shape {

        ShapePath {
            strokeWidth: 0
            strokeColor: 'transparent'
            fillColor: root.mainColor
            strokeStyle: ShapePath.SolidLine
            startX: root.width - 1; startY: 0

            PathLine { x: root.width - 1; y: markerHeight }
            PathLine { x: root.width + 1; y: markerHeight }
            PathLine { x: root.width + 1; y: 0 }
        }
    }
    Image {
        x: -14
        y: 2
        width: 12
        height: 12
        source: "images/mark_loop.svg"
    }
}
