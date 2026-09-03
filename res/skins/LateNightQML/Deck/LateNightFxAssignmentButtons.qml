import QtQuick
import QtQuick.Layouts
import "../LateNightTheme"

Item {
    id: root

    required property string group
    required property bool showFourEffectUnits

    implicitWidth: root.showFourEffectUnits ? 86 : 52
    implicitHeight: 20

    RowLayout {
        anchors.fill: parent
        spacing: 0

        Repeater {
            model: root.showFourEffectUnits ? 4 : 2

            LateNightControlButton {
                required property int index

                Layout.fillHeight: true
                Layout.preferredWidth: root.showFourEffectUnits && index > 0 ? 20 : 26
                activeBackgroundSuffix: "active"
                activeColor: index < 2
                        ? (LateNightTheme.isClassic ? LateNightTheme.effectsUnitColor12 : LateNightTheme.effectsUnitDimColor12)
                        : (LateNightTheme.isClassic ? LateNightTheme.effectsUnitColor34 : LateNightTheme.effectsUnitDimColor34)
                activeOpacity: 1.0
                backgroundSource: LateNightTheme.lateNightAsset(
                        "buttons", index === 0 ? "btn_embedded_library.svg" : "btn_embedded_grid.svg")
                backgroundBorderBottom: 2
                backgroundBorderLeft: index === 0 ? 2 : 1
                backgroundBorderRight: 2
                backgroundBorderTop: 2
                fillMargin: 0
                group: `[EffectRack1_EffectUnit${index + 1}]`
                inactiveColor: LateNightTheme.effectsAssignmentInactiveColor
                inactiveOpacity: 1.0
                label: root.showFourEffectUnits
                        ? (index === 0 ? "FX1" : (index + 1).toString())
                        : "FX" + (index + 1).toString()
                labelColor: isVisuallyActive ? LateNightTheme.effectsAssignmentActiveTextColor : LateNightTheme.effectsAssignmentInactiveTextColor
                iconSource: ""
                key: `group_${root.group}_enable`
                toggleable: true
                useBorderImageBackground: true
            }
        }
    }
}
