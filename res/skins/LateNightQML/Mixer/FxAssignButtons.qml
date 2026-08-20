import Mixxx 1.0 as Mixxx
import QtQuick
import "../LateNightTheme"

Row {
    id: root

    required property string groupName
    readonly property int unitCount: showFourUnitsControl.value > 0 ? 4 : 2

    spacing: 0

    Repeater {
        model: root.unitCount

        Item {
            id: cell

            readonly property color activeColor: cell.index < 2 ? (LateNightTheme.isClassic ? LateNightTheme.effectsUnitColor12 : LateNightTheme.effectsUnitDimColor12) : (LateNightTheme.isClassic ? LateNightTheme.effectsUnitColor34 : LateNightTheme.effectsUnitDimColor34)
            readonly property int buttonWidth: root.unitCount === 2 || cell.index === 0 ? 26 : 20
            required property int index

            height: 20
            implicitHeight: 20
            implicitWidth: buttonWidth
            width: buttonWidth

            Rectangle {
                anchors.fill: parent
                color: assignControl.value > 0 ? cell.activeColor : LateNightTheme.effectsAssignmentInactiveColor
            }
            Image {
                anchors.fill: parent
                fillMode: Image.Stretch
                source: {
                    const suffix = assignControl.value > 0 ? "_active" : "";
                    return LateNightTheme.lateNightAsset("buttons", cell.index === 0 ? "btn_embedded_library" + suffix + ".svg" : "btn_embedded_grid" + suffix + ".svg");
                }
            }
            Text {
                anchors.centerIn: parent
                color: assignControl.value > 0 ? LateNightTheme.effectsAssignmentActiveTextColor : LateNightTheme.effectsAssignmentInactiveTextColor
                font.bold: true
                font.pixelSize: 12
                text: root.unitCount === 2 || cell.index === 0 ? "FX" + (cell.index + 1) : cell.index + 1
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
    Mixxx.ControlProxy {
        id: showFourUnitsControl

        group: "[Skin]"
        key: "show_4effectunits"
    }
}
