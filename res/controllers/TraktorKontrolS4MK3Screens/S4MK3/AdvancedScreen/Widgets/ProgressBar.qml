import QtQuick 2.15

import '../Defines' as Defines

Item {

    id: progressBarContainer

    Defines.Colors { id: colors}
    Defines.Settings { id: settings}

    property color progressBarColorIndicatorLevel: settings.accentColor // set from outside
    property real value: 0.0
    property bool drawAsEnabled: true

    property alias progressBarWidth: progressBar.width
    property alias progressBarHeight: progressBarContainer.height
    property alias progressBarBackgroundColor: progressBar.color // set from outside

    onValueChanged: {
        var val  = Math.max( Math.min(value, 1.0), 0.0)
        valueIndicator.width = val * (progressBar.width - 3)
    }

    height: 6
    width: 80

  // Progress Background
    Rectangle {
        id: progressBar

        anchors.left: parent.left
        anchors.top: parent.top
        height: parent.height
        width: 102 // default value - set from outside

        color: colors.colorWhite09 // set in BottomInfoDetails

    // Progress Level
        Rectangle {
            id: valueIndicator
            width: 0 // set in parent
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            color: progressBarContainer.progressBarColorIndicatorLevel
            visible: drawAsEnabled ? true : false
        }
    // Progress Indicator Thumb
        Rectangle {
            id: indicatorThumb
            color: colors.colorWhite
            width: 2
            height: parent.height
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: valueIndicator.right
            visible: drawAsEnabled ? true : false
        }
    }
}
