import QtQuick 2.15

import '../Widgets' as Widgets
import '../Defines' as Defines

Item {
    id: fxInfoDetails

    property var parameter: ({}) // set from outside
    property bool isOn: false
    property string label: "DRUMLOOP"
    property string sizeState: "small"
    property string buttonLabel: "HP ON"
    property bool fxEnabled: false
    property bool indicatorEnabled: fxEnabled && label.length > 0
    property string finalValue: fxEnabled ? parameter.description : ""
    property string finalLabel: fxEnabled ? label : ""
    property string finalButtonLabel: fxEnabled ? buttonLabel : ""
    property color       barBgColor        // set from outside
    property bool		 isPatternPlayer:	 false

    property alias textColor: colors.colorFontFxHeader

    readonly property int macroEffectChar: 0x00B6
    readonly property bool isMacroFx: (finalLabel.charCodeAt(0) == macroEffectChar)

    width: 80
    height: 45

    Defines.Colors { id: colors }
    Defines.Settings {id: settings}

    // Level indicator for knobs
    Widgets.ProgressBar {
        id: slider
        progressBarHeight: (sizeState == "small") ? 6 : 9
        progressBarWidth: 76
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.topMargin: 3
        anchors.leftMargin: 2

        value: parameter.value
        visible: fxEnabled

        drawAsEnabled: indicatorEnabled

        progressBarBackgroundColor: parent.barBgColor
    }

  // stepped progress bar
    Widgets.StateBar {
        id: slider2
        height: (sizeState == "small") ? 6 : 9
        width: 76
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.leftMargin: 2
        anchors.topMargin: 3

        stateCount: parameter.valueRange.steps
        currentState: (slider2.stateCount - 1.0 + 0.2) * parameter.value // +.2 to make sure we round in the right direction
        visible: parameter.valueRange != undefined && parameter.valueRange.steps > 1 && fxEnabled && label.length > 0
        barBgColor: parent.barBgColor
    }

  // Diverse Elements
    Item {
        id: fxInfoDetailsPanel

        height: 100
        width: parent.width

        Rectangle {
            id: macroIconDetails
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.leftMargin: 4
            anchors.topMargin: 15

            width: 12
            height: 11
            radius: 1
            visible: isMacroFx
            color: colors.colorGrey216

            Text {
                anchors.fill: parent
                anchors.topMargin: -1
                anchors.leftMargin: 1
                text: "M"
                font.pixelSize: fonts.miniFontSize
                color: colors.colorBlack
            }
        }

    // fx name
        Text {
            id: fxInfoSampleName
            font.capitalization: Font.AllUppercase
            text: finalLabel
            color: isPatternPlayer ? colors.colorGreenMint : settings.accentColor
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.topMargin: 8
            font.pixelSize: fonts.scale(13.5)
            anchors.leftMargin: isMacroFx ? 26 : 4
            anchors.rightMargin: 12
            elide: Text.ElideRight
        }

    // value
        Text {
            id: fxInfoValueLarge
            text: finalValue
            font.family: "Pragmatica" // is monospaced
            color: colors.colorWhite
            visible: label.length > 0
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.leftMargin: 4
            font.pixelSize: 15
            anchors.topMargin: 24
        }

    // button
        Rectangle {
            id: fxInfoFilterButton
            width: 30

            color: ( fxEnabled ? (isOn ? (isPatternPlayer ? colors.colorGreenMint : colors.colorIndicatorLevelOrange) : colors.colorBlack) : "transparent" )
            visible: buttonLabel.length > 0
            radius: 1
            anchors.right: parent.right
            anchors.rightMargin: 2
            anchors.top: parent.top
            height: 15
            anchors.topMargin: 26

            Text {
                id: fxInfoFilterButtonText
                font.capitalization: Font.AllUppercase
                text: finalButtonLabel
                color: ( fxEnabled ? (isOn ? colors.colorBlack : colors.colorGrey128) : colors.colorGrey128 )
                font.pixelSize: fonts.miniFontSize
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
            }
        }
    }
}
