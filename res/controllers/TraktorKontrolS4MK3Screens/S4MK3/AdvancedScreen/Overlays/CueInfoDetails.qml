import QtQuick 2.15

import '../Widgets' as Widgets
import '../Defines' as Defines

Item {
    id: fxInfoDetails

    Defines.Settings {id: settings}

    property var parameter: ({description:"Description",value: 0, valueRange: {isDiscrete: true, steps: 1}}) // set from outside
    property bool isOn: false
    property string label: "DRUMLOOP"
    property string buttonLabel: "HP ON"

    property bool		 hideValue:		 true
    property bool		 fxEnabled:		 false

    property bool indicatorEnabled: fxEnabled && label.length > 0
    property string finalValue: ""
    property string finalLabel: ""

    function toInt_round(val) { return parseInt(val+0.5); }

    property alias textColor: colors.colorFontFxHeader

    readonly property int macroEffectChar: 0x00B6
    readonly property bool isMacroFx: (finalLabel.charCodeAt(0) == macroEffectChar)

    width: 0
    height: 45

    Defines.Colors { id: colors }

    // Level indicator for knobs

  // Diverse Elements
    Item {
        id: fxInfoDetailsPanel

        height: 45
        width: parent.width

    // fx name
        Text {
            id: fxInfoSampleName
            font.capitalization: Font.AllUppercase
            text: finalLabel
            color: settings.accentColor
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.topMargin: 2
            font.pixelSize: fonts.scale(13.5)
            anchors.leftMargin: 4
            elide: Text.ElideRight
        }

    // value
        Text {
            id: fxInfoValueLarge
            text: finalValue
            font.family: "Pragmatica" // is monospaced
            color: colors.colorWhite
            visible: true
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.leftMargin: 4
            font.pixelSize: 15
            anchors.topMargin: 22
        }
    }
}
