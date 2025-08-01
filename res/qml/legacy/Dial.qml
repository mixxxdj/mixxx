import QtQuick
import QtQuick.Controls

Dial {
    id: control
    inputMode: Dial.Vertical
    wheelEnabled: true
    implicitWidth: 36
    implicitHeight: 36

    property string color: "#808080"

    background: Image {
        source: "image://svgmodifier/dial/background.svg"
        x: 0
        y: 0
        width: control.width
        height: control.height
    }

    handle: Item {
        readonly property bool trackFromCenter: from === -to
        x: 0
        y: 0
        width: control.width
        height: control.height
        Item {
            x: 0
            y: 0
            width: control.width / 2
            height: control.height
            clip: true
            transform: [
                Rotation {
                    angle: handle.trackFromCenter ? 0 : 40
                    origin.x: control.width / 2
                    origin.y: control.height / 2
                }
            ]
            Image {
                source: "image://svgmodifier/dial/track.svg?#ff0000/" + control.color
                x: 0
                y: 0
                width: control.width
                height: control.height
                visible: !handle.trackFromCenter || control.angle <= 0
                transform: [
                    Rotation {
                        angle: handle.trackFromCenter ? control.angle + 180 : control.angle < 40 ? control.angle - 40 : 0
                        origin.x: control.width / 2
                        origin.y: control.height / 2
                    }
                ]
            }
        }
        Item {
            x: control.width / 2
            y: 0
            width: control.width / 2
            height: control.height
            clip: true
            Image {
                source: "image://svgmodifier/dial/track.svg?#ff0000/" + control.color
                visible: control.angle >= 0
                x: -control.width / 2
                y: 0
                width: control.width
                height: control.height
                transform: [
                    Rotation {
                        angle: control.angle
                        origin.x: control.width / 2
                        origin.y: control.height / 2
                    }
                ]
            }
        }
        Image {
            source: "image://svgmodifier/dial/foreground.svg"
            x: 0
            y: 0
            width: control.width
            height: control.height
        }
        Image {
            x: 0
            y: 0
            source: "image://svgmodifier/dial/handle.svg?#ff0000/" + control.color
            width: control.width
            height: control.height
            transform: [
                Rotation {
                    angle: control.angle
                    origin.x: control.width / 2
                    origin.y: control.height / 2
                }
            ]
        }
    }
}
