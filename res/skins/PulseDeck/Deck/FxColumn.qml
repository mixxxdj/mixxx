pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import Mixxx 1.0 as Mixxx
import "../Effects"
import "../LateNightTheme"

// VirtualDJ style effect section of a deck: the three slots of the effect unit
// the deck is routed to, as compact rows (on/off, knob, effect name).
Item {
    id: root

    required property string group

    readonly property color unitColor: LateNightTheme.deckColorForGroup(root.group)
    // The unit the deck is routed to. While no unit is routed, the decks use
    // the unit shown on their side of the effect rack, so the section is never
    // showing an unrelated unit.
    readonly property int unitNumber: {
        const units = [unit1Enable, unit2Enable, unit3Enable, unit4Enable];
        for (let unit = 0; unit < units.length; ++unit) {
            if (units[unit].value > 0) {
                return unit + 1;
            }
        }
        return LateNightTheme.deckNumberForGroup(root.group) % 2 === 1 ? 1 : 4;
    }
    readonly property string unitGroup: `[EffectRack1_EffectUnit${root.unitNumber}]`

    Mixxx.ControlProxy {
        id: unit1Enable

        group: "[EffectRack1_EffectUnit1]"
        key: `group_${root.group}_enable`
    }
    Mixxx.ControlProxy {
        id: unit2Enable

        group: "[EffectRack1_EffectUnit2]"
        key: `group_${root.group}_enable`
    }
    Mixxx.ControlProxy {
        id: unit3Enable

        group: "[EffectRack1_EffectUnit3]"
        key: `group_${root.group}_enable`
    }
    Mixxx.ControlProxy {
        id: unit4Enable

        group: "[EffectRack1_EffectUnit4]"
        key: `group_${root.group}_enable`
    }
    ColumnLayout {
        anchors.fill: parent
        spacing: 1

        Repeater {
            model: 3

            EffectSlot {
                required property int index

                Layout.fillHeight: true
                Layout.fillWidth: true
                effectNumber: index + 1
                expanded: false
                unitColor: root.unitColor
                unitGroup: root.unitGroup
                unitNumber: root.unitNumber
            }
        }
    }
}
