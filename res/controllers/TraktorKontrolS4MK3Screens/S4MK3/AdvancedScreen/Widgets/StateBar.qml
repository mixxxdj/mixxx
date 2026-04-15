import QtQuick 2.15

import '../Defines' as Defines

// StateBar fits 'state count' elements into a bar of a given width and spacing. Take care that 'width > stateCount*spacing'
Item {
    id: stateBarContainer

    property int spacing: 2 // default value. set from outside
    property int stateCount: 5 // default value. set from outside
    property int currentState: 2 // default value. set from outside
    property color barColor: colors.colorIndicatorLevelOrange // default value. set from outside
    property color barBgColor: colors.colorGrey24 // default value. set from outside

    property alias stateBarHeight: stateBarContainer.height
    readonly property real stateBarWidth: width/stateCount - spacing

    Defines.Colors { id: colors}

    Row {
        id: boxRow
        anchors.fill: parent
        anchors.leftMargin: 0.5*stateBarContainer.spacing
        spacing: stateBarContainer.spacing
        Repeater {
            model: stateCount
            Rectangle {
                width: stateBarWidth
                height: stateBarHeight
                color: (index == currentState) ? barColor : barBgColor
            }
        }
    }
}
