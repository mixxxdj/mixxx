import QtQuick 2.15

import '../Widgets' as Widgets
import '../Defines' as Defines

Item {
    id: fxInfoDetails

    Defines.Settings {id: settings}

    property var parameter: ({description:"Description",value: 0}) // set from outside
    property bool isOn: false
    property string label: "DRUMLOOP"
    property string sizeState: "small"
    property string buttonLabel: "HP ON"
    property bool fxEnabled: false
    property bool		 zoom:			 true
    property bool		 hideButton:		 true
    property bool		 hideValue:		 true

    property bool indicatorEnabled: fxEnabled && label.length > 0
    property string finalValue: zoom ? (((10 - parameter.value) / 9)*100).toFixed(2)+"%" : toInt_round(parameter.value*100).toString() + "%"
    property string finalLabel: fxEnabled ? label : ""
    property string finalButtonLabel: "ON"
    property color       barBgColor        // set from outside

    function toInt_round(val) { return parseInt(val+0.5); }

    property alias textColor: colors.colorFontFxHeader

    readonly property int macroEffectChar: 0x00B6
    readonly property bool isMacroFx: (finalLabel.charCodeAt(0) == macroEffectChar)

    readonly property var valueRange: parameter.valueRange || {}

    width: 80
    height: 45

    Defines.Colors { id: colors }

    // Level indicator for knobs
    Widgets.ProgressBar {
        id: slider
        progressBarHeight: (sizeState == "small") ? 6 : 9
        progressBarWidth: 76
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.topMargin: 3
        anchors.leftMargin: 2

        value: label == "ZOOM" ? (10 - parameter.value) / 9 : parameter.value
        visible: !(valueRange.isDiscrete && fxEnabled)

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

        stateCount: valueRange.steps || 0
        currentState: (valueRange.steps - 1.0 + 0.2) * parameter.value // +.2 to make sure we round in the right direction
        visible: !slider.visible
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
            anchors.topMargin: 50

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
            color: settings.accentColor
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.topMargin: 40
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
            visible: (label.length > 0) && !hideValue
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.leftMargin: 4
            font.pixelSize: 15
            anchors.topMargin: 22
        }

    // button
        Rectangle {
            id: fxInfoFilterButton
            width: 30

            color: ( fxEnabled ? (isOn ? colors.colorIndicatorLevelOrange : colors.colorBlack) : "transparent" )
            visible: (buttonLabel.length > 0) && !hideButton
            radius: 1
            anchors.right: parent.right
            anchors.rightMargin: 2
            anchors.top: parent.top
            height: 15
            anchors.topMargin: 24

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
