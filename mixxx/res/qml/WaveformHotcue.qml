import "." as Skin
import Mixxx 1.0 as Mixxx
import QtQuick 2.15
import QtQuick.Shapes 1.4
import QtQuick.Window 2.15

import QtQuick.Controls 2.15

import "Theme"

Item {
    id: root

    required property int hotcueNumber
    required property string group
    required property string label
    property bool isLoop: false

    Skin.Hotcue {
        id: hotcue

        group: root.group
        hotcueNumber: root.hotcueNumber
    }

    property real markerHeight: root.height
    property color labelColor: Theme.waveformMarkerLabel
    property real radiusSize: 4
    property string hotcueLabel: label != "" ? `${hotcueNumber}: ${label}` : `${hotcueNumber}`

    FontMetrics {
        id: fontMetrics
        font.family: Theme.fontFamily
    }

    property rect contentRect: fontMetrics.tightBoundingRect(hotcueLabel)

    Rectangle {
        visible: root.isLoop
        anchors.fill: parent
        color: Qt.alpha(hotcue.color, 0.3)
    }

    Shape {
        ShapePath {
            strokeWidth: 0
            strokeColor: 'transparent'
            fillColor: hotcue.color
            strokeStyle: ShapePath.SolidLine
            startX: -1; startY: 0

            PathLine { x: 1; y: 0 }
            PathLine { x: 1; y: markerHeight - 16 }
            PathLine { x: 8 - radiusSize + contentRect.width; y: markerHeight - 16 }
            PathArc {
                x: 8 + contentRect.width
                y: markerHeight - 16 + radiusSize
                radiusX: radiusSize; radiusY: radiusSize
                direction: PathArc.Clockwise
            }
            PathLine { x: 8 + contentRect.width; y: markerHeight - radiusSize }
            PathArc {
                x: 8 - radiusSize + contentRect.width
                y: markerHeight
                radiusX: radiusSize; radiusY: radiusSize
                direction: PathArc.Clockwise
            }
            PathLine { x: -1; y: markerHeight }
            PathLine { x: -1; y: 0 }
        }
    }
    Shape {
        visible: root.isLoop
        ShapePath {
            strokeWidth: 0
            strokeColor: 'transparent'
            fillColor: hotcue.color
            strokeStyle: ShapePath.SolidLine
            startX: root.width - 1; startY: 0

            PathLine { x: root.width - 1; y: markerHeight }
            PathLine { x: root.width + 1; y: markerHeight }
            PathLine { x: root.width + 1; y: 0 }
        }
    }
    Shape {
        ShapePath {
            fillColor: labelColor
            strokeColor: labelColor
            PathText {
                x: 3
                y: markerHeight - 13
                font.family: Theme.fontFamily
                font.pixelSize: 13
                font.weight: Font.Medium
                text: hotcueLabel
            }
        }
    }
}
