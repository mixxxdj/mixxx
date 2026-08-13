import Mixxx 1.0 as Mixxx
import QtQuick
import "../LateNightTheme"

Rectangle {
    id: root

    readonly property real firstPairHeight: Math.max(unit1.implicitHeight, unit2.implicitHeight)
    readonly property int rowSpacing: LateNightTheme.isClassic ? 4 : 3
    readonly property real secondPairHeight: showFourUnits ? Math.max(unit3.implicitHeight, unit4.implicitHeight) : 0
    readonly property bool showFourUnits: showFourUnitsControl.value > 0

    color: LateNightTheme.effectsRackGutterColor
    implicitHeight: firstPairHeight + secondPairHeight + (showFourUnits ? rowSpacing : 0) + rowSpacing * 2

    Item {
        id: pair12

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.topMargin: root.rowSpacing
        height: root.firstPairHeight

        EffectUnit {
            id: unit1

            height: implicitHeight
            unitNumber: 1
            width: Math.round((parent.width - 4) / 2)
            x: 0
            y: 0
        }
        RackFiller {
            height: Math.max(0, parent.height - y)
            visible: height > 0
            width: unit1.width - 1
            x: unit1.x
            y: unit1.height + root.rowSpacing
        }
        EffectUnit {
            id: unit2

            height: implicitHeight
            unitNumber: 2
            width: parent.width - x
            x: unit1.width + 4
            y: 0
        }
        RackFiller {
            height: Math.max(0, parent.height - y)
            visible: height > 0
            width: unit2.width - 1
            x: unit2.x + 1
            y: unit2.height + root.rowSpacing
        }
    }
    Item {
        id: pair34

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: pair12.bottom
        anchors.topMargin: root.rowSpacing
        height: root.secondPairHeight
        visible: root.showFourUnits

        EffectUnit {
            id: unit3

            height: implicitHeight
            unitNumber: 3
            width: Math.round((parent.width - 4) / 2)
            x: 0
            y: 0
        }
        RackFiller {
            height: Math.max(0, parent.height - y)
            visible: height > 0
            width: unit3.width - 1
            x: unit3.x
            y: unit3.height + root.rowSpacing
        }
        EffectUnit {
            id: unit4

            height: implicitHeight
            unitNumber: 4
            width: parent.width - x
            x: unit3.width + 4
            y: 0
        }
        RackFiller {
            height: Math.max(0, parent.height - y)
            visible: height > 0
            width: unit4.width - 1
            x: unit4.x + 1
            y: unit4.height + root.rowSpacing
        }
    }
    Mixxx.ControlProxy {
        id: showFourUnitsControl

        group: "[Skin]"
        key: "show_4effectunits"
    }

    component RackFiller: Rectangle {
        border.color: "#111111"
        border.width: 1
        color: LateNightTheme.effectsFillerColor
        radius: LateNightTheme.isClassic ? 2 : 1

        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            color: LateNightTheme.isClassic ? "#222222" : "#1c1c1c"
            height: 1
        }
        Rectangle {
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.top: parent.top
            color: LateNightTheme.isClassic ? "#222222" : "#191919"
            width: 1
        }
        Rectangle {
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            color: LateNightTheme.isClassic ? "#111111" : "#020202"
            height: 1
        }
    }
}
