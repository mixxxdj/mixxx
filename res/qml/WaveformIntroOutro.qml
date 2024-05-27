import Mixxx 1.0 as Mixxx
import QtQuick 2.15
import QtQuick.Shapes 1.4
import QtQuick.Window 2.15

import QtQuick.Controls 2.15

import "Theme"

Item {
    id: root

    property color mainColor: Theme.waveformMarkerIntroOutroColor
    property bool isIntro: true

    property real markerHeight: root.height
    property real radiusSize: 4

    Rectangle {
        anchors.fill: parent
        color: Qt.alpha(root.mainColor, 0.1)
    }

    Shape {
        ShapePath {
            strokeWidth: 0
            strokeColor: 'transparent'
            fillColor: root.mainColor
            strokeStyle: ShapePath.SolidLine
            startX: -1; startY: 0

            PathLine { x: 16 - radiusSize; y: 0 }
            PathArc {
                x: 16
                y: radiusSize
                radiusX: radiusSize; radiusY: radiusSize
                direction: PathArc.Clockwise
            }
            PathLine { x: 16; y: 16 - radiusSize }
            PathArc {
                x: 16 - radiusSize
                y: 16
                radiusX: radiusSize; radiusY: radiusSize
                direction: PathArc.Clockwise
            }
            PathLine { x: 1; y: 16 }
            PathLine { x: 1; y: markerHeight }
            PathLine { x: -1; y: markerHeight }
        }
    }
    Shape {
        visible: root.width != 0
        ShapePath {
            strokeWidth: 0
            strokeColor: 'transparent'
            fillColor: root.mainColor
            strokeStyle: ShapePath.SolidLine
            startX: root.width - 1; startY: 0

            PathLine { x: root.width - 16 + radiusSize; y: 0 }
            PathArc {
                x: root.width - 16
                y: radiusSize
                radiusX: radiusSize; radiusY: radiusSize
                direction: PathArc.Counterclockwise
            }
            PathLine { x: root.width - 16; y: 16 - radiusSize }
            PathArc {
                x: root.width - 16 + radiusSize
                y: 16
                radiusX: radiusSize; radiusY: radiusSize
                direction: PathArc.Counterclockwise
            }
            PathLine { x: root.width -1; y: 16 }
            PathLine { x: root.width -1; y: markerHeight }
            PathLine { x: root.width + 1; y: markerHeight }
            PathLine { x: root.width + 1; y: 0 }
        }
    }
    Image {
        x: 2
        y: 2
        width: 12
        height: 12
        source: `images/mark_${(root.isIntro ? 'intro' : 'outro')}.svg`
    }
    Image {
        visible: root.width != 0
        x: root.width - 14
        y: 2
        width: 12
        height: 12
        source: `images/mark_${(root.isIntro ? 'intro' : 'outro')}.svg`
    }
}
