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
        onPressed:  root.handleInput(event)
        onReleased: root.handleInput(event)
        onHovered:  root.handleInput(event)
        onWheel:    root.handleInput(event)
    }
}