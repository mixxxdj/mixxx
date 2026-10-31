import QtQuick 2.15

import '../Widgets' as Widgets
import '../Defines' as Defines

Item {
    id: fxInfoDetails

    property var parameter: ({description:"Description",value: 0, valueRange: {isDiscrete: true, steps: 1}}) // set from outside
    property string label: "DRUMLOOP"

    property alias textColor: colors.colorFontFxHeader
    property bool header: false
    property int effectID: 0
    property int fxUnit: 1

  // AppProperty {id: slot1; path: "app.traktor.fx." + fxUnit + ".select.1"}
    QtObject {
        id: slot1
        property string description: "Description"
        property var value: 0
    }
  // AppProperty {id: slot2; path: "app.traktor.fx." + fxUnit + ".select.2"}
    QtObject {
        id: slot2
        property string description: "Description"
        property var value: 0
    }
  // AppProperty {id: slot3; path: "app.traktor.fx." + fxUnit + ".select.3"}
    QtObject {
        id: slot3
        property string description: "Description"
        property var value: 0
    }

    width: 0
    height: 20

    Defines.Colors { id: colors }
    Defines.Settings {id: settings}

    // Level indicator for knobs

  // Diverse Elements
    Item {
        id: fxInfoDetailsPanel

        height: 20
        width: parent.width

    // fx name
        Text {
            id: fxInfoSampleName
            font.capitalization: Font.AllUppercase
            text: label
            color: header ? settings.accentColor : (slot1.value == effectID || slot2.value == effectID || slot3.value == effectID ? "lime" : "white")
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.topMargin: 2
            font.pixelSize: fonts.scale(13.5)
            anchors.leftMargin: 4
            elide: Text.ElideRight
        }
    }
}
