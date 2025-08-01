import QtQuick
import QtQuick.Controls

Slider {
    id: control
    leftPadding: 0
    rightPadding: 0
    topPadding: 0
    bottomPadding: 0
    implicitWidth: implicitBackgroundWidth
    implicitHeight: implicitBackgroundHeight

    property url backgroundSource
    property url handleSource

    background: Image {
        source: backgroundSource
        Rectangle {
            readonly property real offset: horizontal ? control.leftPadding + 6 : control.topPadding + 6
            readonly property real extreme: horizontal ? control.leftPadding + 6 : control.height - control.bottomPadding - 6
            readonly property real center: horizontal ? control.width / 2 : control.height / 2
            readonly property real range: horizontal ? control.availableWidth - 12 : control.availableHeight - 12
            readonly property real trackFrom: control.from === -control.to ? center : extreme
            readonly property real trackTo: offset + control.visualPosition * range
            readonly property real trackMin: trackFrom < trackTo ? trackFrom : trackTo
            readonly property real trackMax: trackFrom > trackTo ? trackFrom : trackTo
            color: "#257b82"
            x: horizontal ? trackMin : (control.width - width) / 2
            y: vertical ? trackMin : (control.height - height) / 2
            width: horizontal ? trackMax - trackMin : 2
            height: vertical ? trackMax - trackMin : 2
        }
    }

    handle: Image {
        source: handleSource
        x: horizontal ? control.leftPadding + control.visualPosition * (control.availableWidth - width) : control.leftPadding + control.availableWidth / 2 - width / 2
        y: vertical ? control.topPadding + control.visualPosition * (control.availableHeight - height) : control.topPadding + control.availableHeight / 2 - height / 2
    }
}
