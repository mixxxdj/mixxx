import QtQuick 2.15

import '../Widgets' as Widgets
import '../Defines' as Defines

Item {
    id: fxInfoDetails

    property string label: ""
    property bool header: false
    property string color: "white"

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
            color: header ? settings.accentColor : fxInfoDetails.color
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.topMargin: 0
            font.pixelSize: fonts.scale(18)
            anchors.leftMargin: 4
            elide: Text.ElideRight
            horizontalAlignment: header ? Text.AlignLeft : Text.AlignHCenter
        }
    }
}
