import QtQuick 2.0
import Mixxx 1.0 as Mixxx

Item {
    id: root
    visible: false
    anchors.fill: parent
    property bool paused: false

    // Watch deck 1 play state
    Mixxx.ControlProxy {
        id: deck1Play
        group: "[Channel1]"
        key: "play"

        onValueChanged: {
            root.paused = (value === 0);
        }
    }

    // Animation for showing/hiding the vinyl
    Behavior on opacity {
        NumberAnimation { duration: 300 }
    }

    opacity: paused ? 1.0 : 0.0

    // Only spin when visible — saves CPU/battery on mobile
    visible: opacity > 0

    Canvas {
        id: vinylCanvas
        anchors.centerIn: parent
        width: 200
        height: 200
        onPaint: {
            var ctx = getContext("2d")
            ctx.reset()
            ctx.clearRect(0, 0, width, height)

            // Draw outer ring (vinyl record)
            ctx.lineWidth = 10
            ctx.strokeStyle = "#333"
            ctx.beginPath()
            ctx.arc(width/2, height/2, width/2 - 5, 0, 2 * Math.PI)
            ctx.stroke()

            // Draw inner circle (label)
            ctx.fillStyle = "#222"
            ctx.beginPath()
            ctx.arc(width/2, height/2, width/4, 0, 2 * Math.PI)
            ctx.fill()

            // Draw a line to represent the arm (optional)
            ctx.strokeStyle = "#fff"
            ctx.lineWidth = 2
            ctx.beginPath()
            ctx.moveTo(width/2, height/2)
            ctx.lineTo(width/2, height/2 - width/2 + 10)
            ctx.stroke()
        }
    }
}