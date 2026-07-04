import QtQuick 2.0
import QtQuick.Window 2.1

Item {
    id: root
    property bool externalMouseConnected: false
    property int timerInterval: 2000 // milliseconds

    Timer {
        id: mouseTimeout
        interval: root.timerInterval
        repeat: false
        onTriggered: {
            root.externalMouseConnected = false
        }
    }

    function handleInput(event) {
        // event.source: 0=NotSynthesized, 1=Mouse, 2=TouchScreen, 3=TouchPad, 4=VirtualGroup, 5=Pen
        if (event.source === 1) { // Mouse
            root.externalMouseConnected = true
        }
        mouseTimeout.restart()
    }

    // MouseArea to catch all mouse and touch events
    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        acceptedButtons: Qt.AllButtons
        // Accept touch events by setting mouse.acceptedButtons? Actually, MouseArea handles touch as mouse events with source set accordingly.
        onPressed:  root.handler.mouse(event)
        onReleased: root.handler.mouse(event)
        onHovered:  root.handler.mouse(event)
        onWheel:    root.handler.mouse(event)
        // For touch events that are not captured as mouse events? We rely on the above.
    }

    // We need to expose the handler to the MouseArea
    property variant handler: {
        return {
            mouse: function(event) {
                root.handleInput(event)
            }
        }
    }
}