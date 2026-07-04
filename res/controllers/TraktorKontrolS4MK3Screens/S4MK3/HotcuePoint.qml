/*
This module is used to define markers element as render over the the overview waveform.
When this is written, Mixxx QML doesn't have waveform overview marker ready to be used, so this
is an attempt to provide fully functional markers for the controller screen, while the Mixxx QML
interface is still being worked on.
Consider replacing this with native overview marker in the future.
*/
import QtQuick 2.15
import QtQuick.Shapes 1.4
import QtQuick.Window 2.15

import QtQuick.Controls 2.15

import Mixxx 1.0 as Mixxx

Item {
    required property real position
    required property int type

    property int number: 1
    property color color: 'blue'

    enum Type {
        OneShot,
        Loop,
        IntroIn,
        IntroOut,
        OutroIn,
        OutroOut,
        LoopIn,
        LoopOut
    }

    property variant typeWithNumber: [
                                      HotcuePoint.Type.OneShot,
                                      HotcuePoint.Type.Loop
    ]

    x: position * (Window.width - 16)
    width: 21

    // One shot
    Shape {
        visible: type == HotcuePoint.Type.OneShot
        anchors.fill: parent
        antialiasing: true

        ShapePath {
            strokeWidth: 1
            strokeColor: Qt.rgba(0, 0, 0, 0.5)
            fillColor: color
            strokeStyle: ShapePath.SolidLine
            // dashPattern: [ 1, 4 ]
            startX: 0; startY: 0

            PathLine { x: 12; y: 0 }
            PathLine { x: 18; y: 6 }
            PathLine { x: 18; y: 7 }
            PathLine { x: 12; y: 13 }
            PathLine { x: 2; y: 13 }
            PathLine { x: 2; y: 80 }
            PathLine { x: 0; y: 80 }
            PathLine { x: 0; y: 0 }
        }
    }

    // Intro/Outro entry marker
    Shape {
        visible: type == HotcuePoint.Type.IntroIn || type == HotcuePoint.Type.OutroIn
        anchors.fill: parent
        antialiasing: true

        ShapePath {
            strokeWidth: 1
            strokeColor: Qt.rgba(0, 0, 0, 0.5)
            fillColor: "#6e6e6e"
            strokeStyle: ShapePath.SolidLine
            // dashPattern: [ 1, 4 ]
            startX: 0; startY: 0

            PathLine { x: 11; y: 0 }
            PathLine { x: 2; y: 13 }
            PathLine { x: 2; y: 80 }
            PathLine { x: 0; y: 80 }
            PathLine { x: 0; y: 0 }
        }
    }

    // Intro/Outro exit marker
    Shape {
        visible: type == HotcuePoint.Type.IntroOut || type == HotcuePoint.Type.OutroOut
        anchors.fill: parent
        antialiasing: true

        ShapePath {
            strokeWidth: 1
            strokeColor: Qt.rgba(0, 0, 0, 0.5)
            fillColor: "#6e6e6e"
            strokeStyle: ShapePath.SolidLine
            // dashPattern: [ 1, 4 ]
            startX: 2; startY: 0

            PathLine { x: 0; y: 0 }
            PathLine { x: 0; y: 67 }
            PathLine { x: -9; y: 80 }
            PathLine { x: 2; y: 80 }
            PathLine { x: 2; y: 0 }
        }
    }

    // Loop
    Shape {
        visible: type == HotcuePoint.Type.Loop
        anchors.fill: parent
        antialiasing: true

        ShapePath {
            strokeWidth: 1
            strokeColor: Qt.rgba(0, 0, 0, 0.5)
            fillColor: "#6ef36e"
            strokeStyle: ShapePath.SolidLine
            // dashPattern: [ 1, 4 ]
            startX: 13; startY: 0

            PathArc {
                x: 2; y: 13
                radiusX: 9; radiusY: 9
                direction: PathArc.Clockwise
            }
            PathLine { x: 2; y: 80 }
            PathLine { x: 0; y: 80 }
            PathLine { x: 0; y: 0 }
            PathLine { x: 21; y: 0 }
        }
    }

    // Loop in
    Shape {
        visible: type == HotcuePoint.Type.LoopIn
        anchors.fill: parent
        antialiasing: true

        ShapePath {
            strokeWidth: 1
            strokeColor: Qt.rgba(0, 0, 0, 0.5)
            fillColor: "#6ef36e"
            strokeStyle: ShapePath.SolidLine
            // dashPattern: [ 1, 4 ]
            startX: 0; startY: 0

            PathLine { x: 8; y: 0 }
            PathLine { x: 2; y: 10 }
            PathLine { x: 2; y: 80 }
            PathLine { x: 0; y: 80 }
            PathLine { x: 0; y: 0 }
        }
    }

    // Loop out
    Shape {
        visible: type == HotcuePoint.Type.LoopOut
        anchors.fill: parent
        antialiasing: true

        ShapePath {
            strokeWidth: 1
            strokeColor: Qt.rgba(0, 0, 0, 0.5)
            fillColor: "#6ef36e"
            strokeStyle: ShapePath.SolidLine
            // dashPattern: [ 1, 4 ]
            startX: 2; startY: 0

            PathLine { x: -6; y: 0 }
            PathLine { x: 0; y: 10 }
            PathLine { x: 0; y: 80 }
            PathLine { x: 2; y: 80 }
            PathLine { x: 2; y: 0 }
        }
    }

    Shape {
        visible: type in typeWithNumber
        anchors.fill: parent
        antialiasing: true

        ShapePath {
            fillColor: "black"
            strokeColor: "black"
            PathText {
                x: 4
                y: 3
                font.family: "Arial"
                font.pixelSize: 11
                font.weight: Font.Medium
                text: `${number}`
            }
        }
    }
}
