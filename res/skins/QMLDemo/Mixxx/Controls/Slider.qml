import Mixxx 0.1 as Mixxx
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Shapes 1.12

Item {
    id: root

    property alias group: control.group
    property alias key: control.key
    property alias handle: handleRect.data
    property alias background: background.data
    property bool bar: false
    property real barMargin: 0
    property alias barColor: barPath.strokeColor
    property alias barWidth: barPath.strokeWidth
    property real barStart: 0

    Slider {
        id: slider

        anchors.fill: parent
        orientation: Qt.Vertical
        wheelEnabled: true
        value: control.parameter
        onMoved: control.parameter = value

        Mixxx.ControlProxy {
            id: control
        }

        handle: Rectangle {
            id: handleRect

            x: slider.leftPadding + slider.availableWidth / 2 - childrenRect.width / 2
            y: slider.visualPosition * (slider.height - childrenRect.height)
        }

        background: Rectangle {
            anchors.fill: parent
            color: "transparent"

            Item {
                id: background

                anchors.fill: parent
            }

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
                        y: root.height - root.barMargin - (root.height - 2 * root.barMargin) * control.parameter
                    }

                }

            }

        }

    }

}
