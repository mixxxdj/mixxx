import Mixxx 1.0 as Mixxx
import QtQuick
import "../LateNightTheme"

Row {
    id: root

    required property string groupName

    spacing: 0

    Repeater {
        model: 4

        Item {
            id: cell

            readonly property bool active: assignControl.value > 0
            readonly property color activeColor: cell.index < 2
                    ? (LateNightTheme.isClassic ? LateNightTheme.effectsUnitColor12 : LateNightTheme.effectsUnitDimColor12)
                    : (LateNightTheme.isClassic ? LateNightTheme.effectsUnitColor34 : LateNightTheme.effectsUnitDimColor34)
            readonly property int buttonWidth: cell.index === 0 ? 26 : 20
            readonly property color fillColor: cell.active ? cell.activeColor : LateNightTheme.mixerFxAssignInactiveColor
            required property int index

            height: 20
            implicitHeight: 20
            implicitWidth: buttonWidth
            width: buttonWidth

            Rectangle {
                anchors.fill: parent
                color: cell.fillColor
            }
            Image {
                anchors.fill: parent
                fillMode: Image.Stretch
                source: {
                    const suffix = cell.active ? "_active" : "";
                    return LateNightTheme.lateNightButton(cell.index === 0 ? "btn_embedded_library" + suffix + ".svg" : "btn_embedded_grid" + suffix + ".svg");
                }
            }
            Text {
                anchors.centerIn: parent
                color: cell.active
                        ? (LateNightTheme.isClassic ? LateNightTheme.deckActiveButtonTextColor : LateNightTheme.mixerControlTextColor)
                        : LateNightTheme.mixerFxAssignInactiveTextColor
                font.bold: true
                font.family: "Open Sans"
                font.pixelSize: 10
                text: cell.index === 0 ? "FX1" : cell.index + 1
            }
            TapHandler {
                onTapped: assignControl.value = assignControl.value > 0 ? 0 : 1
            }
            Mixxx.ControlProxy {
                id: assignControl

                group: "[EffectRack1_EffectUnit" + (cell.index + 1) + "]"
                key: "group_" + root.groupName + "_enable"
            }
        }
    }
}
