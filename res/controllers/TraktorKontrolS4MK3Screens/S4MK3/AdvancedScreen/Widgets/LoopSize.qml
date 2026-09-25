import QtQuick 2.15

import Mixxx 1.0 as Mixxx

import '../Defines' as Defines

Item {
    anchors.fill: parent

    Defines.Colors {id: colors}
    Defines.Durations { id: durations }

    Mixxx.ControlProxy {
        group: `[Channel${parent.deckId}]`
        key: "beatloop_size"
        id: loopSize
        property string description: "Description"
    }

    property int deckId: 0

    property color loopActiveColor: colors.cueColors[settings.cueLoopColor]
    property color loopDimmedColor: colors.cueColorsDark[settings.cueLoopColor]

    Rectangle {
        id: loopSizeBackground
        width: 40
        height: width
        radius: width * 0.5
        opacity: loopActiveBlinkTimer.blink ? 0.25 : 1
        color: deckInfo.loopActive ? (loopActiveBlinkTimer.blink ? loopActiveColor : (settings.loopActiveRedFlash ? colors.colorRed : loopDimmedColor))
        : deckInfo.loopActive ? (deckInfo.shift ? loopDimmedColor : loopActiveColor)
        : deckInfo.shift ? colors.colorDeckDarkGrey : colors.colorDeckGrey
        Behavior on opacity { NumberAnimation { duration: durations.mainTransitionSpeed; easing.type: Easing.Linear} }
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        Rectangle {
            id: loopLengthBorder
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            width: loopSizeBackground.width -2
            height: width
            radius: width * 0.5
            color: "transparent"
            border.color: loopActiveColor
            border.width: 2
        }
    }

    Text {
        text: loopSize.value < 1/8 ? `/${1 / loopSize.value}` : loopSize.value < 1 ? `1/${1 / loopSize.value}` : `${loopSize.value}`
        color: deckInfo.loopActive ? "black" : ( deckInfo.shift ? colors.colorDeckGrey : colors.defaultTextColor )
        font.pixelSize: fonts.extraLargeValueFontSize
        font.family: "Pragmatica"
        anchors.fill: loopSizeBackground
        anchors.rightMargin: 2
        anchors.topMargin: 1
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        onTextChanged: {
            if (loopSize.value < 1) {
                font.pixelSize = 18
            } else if ( loopSize.value > 8 ) {
                font.pixelSize = 24
            } else {
                font.pixelSize = 25
            }
        }
    }

    Timer {
        id: loopActiveBlinkTimer
        property bool blink: false

        interval: 333
        repeat: true
        running: deckInfo.loopActive

        onTriggered: {
            blink = !blink;
        }

        onRunningChanged: {
            blink = running;
        }
    }
}
