import Mixxx 1.0 as Mixxx
import QtQuick 2.15
import QtQuick.Shapes 1.4
import QtQuick.Window 2.15

import QtQuick.Controls 2.15

import "Theme"

Item {
    id: root

    property color color: Theme.waveformMarkerDefault

    property real markerHeight: root.height
    property color labelColor: Theme.waveformMarkerLabel
    property real radiusSize: 4
    property string cueLabel: qsTr("CUE")

    FontMetrics {
        id: fontMetrics
        font.family: Theme.fontFamily
    }

    property rect contentRect: fontMetrics.tightBoundingRect(cueLabel)

    Shape {
        ShapePath {
            strokeWidth: 0
            strokeColor: 'transparent'
            fillColor: color
            strokeStyle: ShapePath.SolidLine
            startX: -1; startY: 0

            PathLine { x: 8; y: 0 }
            PathLine { x: 8 - radiusSize + contentRect.width; y: 0 }
            PathArc {
                x: 8 + contentRect.width
                y: radiusSize
                radiusX: radiusSize; radiusY: radiusSize
                direction: PathArc.Clockwise
            }
            PathLine { x: 8 + contentRect.width; y: 16 - radiusSize }
            PathArc {
                x: 8 - radiusSize + contentRect.width
                y: 16
                radiusX: radiusSize; radiusY: radiusSize
                direction: PathArc.Clockwise
            }
            PathLine { x: 8; y: 16 }
            PathLine { x: 2; y: 16 }
            PathLine { x: 2; y: markerHeight }
            PathLine { x: -1; y: markerHeight }
            PathLine { x: -1; y: 0 }
        }
    }
    Shape {
        ShapePath {
            fillColor: labelColor
            strokeColor: labelColor
            PathText {
                x: 3
                y: 3
                font.family: Theme.fontFamily
                font.pixelSize: 13
                font.weight: Font.Bold
                text: cueLabel
            }
        }
    }
}
