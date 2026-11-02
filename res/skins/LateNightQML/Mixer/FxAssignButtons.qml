pragma ComponentBehavior: Bound

import Mixxx 1.0 as Mixxx
import QtQuick
import "../LateNightTheme"

Row {
    id: root

    readonly property int assignmentWidth: root.effectUnitCount === 4 ? 86 : 52
    readonly property int effectUnitCount: showFourEffectUnitsControl.value > 0 ? 4 : 2
    required property string groupName
    readonly property bool micAux: groupName.startsWith("[Microphone") || groupName.startsWith("[Auxiliary")

    spacing: 0

    Repeater {
        model: root.effectUnitCount

        Item {
            id: cell

            readonly property bool active: assignControl.value > 0
            readonly property color activeColor: cell.index < 2 ? (LateNightTheme.isClassic ? LateNightTheme.effectsUnitColor12 : LateNightTheme.effectsUnitDimColor12) : (LateNightTheme.isClassic ? LateNightTheme.effectsUnitColor34 : LateNightTheme.effectsUnitDimColor34)
            readonly property int buttonWidth: root.effectUnitCount === 4 && cell.index > 0 ? 20 : 26
            readonly property color fillColor: cell.active ? cell.activeColor : LateNightTheme.mixerFxAssignInactiveColor
            readonly property url buttonSource: LateNightTheme.lateNightButton((cell.index === 0 ? "btn_embedded_library" : "btn_embedded_grid") + (cell.active ? "_active" : "") + ".svg")
            required property int index

            height: 20
            implicitHeight: 20
            implicitWidth: buttonWidth
            width: buttonWidth

            Rectangle {
                anchors.fill: parent
                color: cell.fillColor
            }
            BorderImage {
                anchors.fill: parent
                border.bottom: 2
                border.left: cell.index === 0 ? 2 : 1
                border.right: 2
                border.top: 2
                source: cell.buttonSource
                visible: root.micAux
            }
            Image {
                anchors.fill: parent
                fillMode: Image.Stretch
                source: cell.buttonSource
                visible: !root.micAux
            }
            Text {
                anchors.centerIn: parent
                color: cell.active ? (root.micAux || LateNightTheme.isClassic ? LateNightTheme.deckActiveButtonTextColor : LateNightTheme.mixerControlTextColor) : LateNightTheme.mixerFxAssignInactiveTextColor
                font.bold: true
                font.family: "Open Sans"
                font.pixelSize: root.micAux ? 11 : 10
                renderType: root.micAux ? Text.NativeRendering : Text.QtRendering
                text: root.effectUnitCount === 4 ? (cell.index === 0 ? (root.micAux ? " FX 1" : "FX1") : cell.index + 1) : (root.micAux ? " FX" : "FX") + (cell.index + 1)
            }
            MouseArea {
                acceptedButtons: Qt.LeftButton
                anchors.fill: parent
                enabled: root.micAux

                onPressed: assignControl.value = assignControl.value > 0 ? 0 : 1
            }
            TapHandler {
                enabled: !root.micAux

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
        id: showFourEffectUnitsControl

        group: "[Skin]"
        key: "show_4effectunits"
    }
}
