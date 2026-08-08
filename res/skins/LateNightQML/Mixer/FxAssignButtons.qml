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

            readonly property int buttonWidth: cell.index === 0 ? 26 : 20
            required property int index

            height: 20
            implicitHeight: 20
            implicitWidth: buttonWidth
            width: buttonWidth

            Rectangle {
                anchors.fill: parent
                color: assignControl.value > 0 ? "#333335" : LateNightTheme.deckEmbeddedButtonInactiveColor
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
                color: assignControl.value > 0 ? LateNightTheme.mixerControlTextColor : LateNightTheme.mixerDimTextColor
                font.bold: true
                font.pixelSize: 12
                text: cell.index === 0 ? "FX1" : cell.index + 1
            }
            MouseArea {
                anchors.fill: parent

                onClicked: assignControl.value = assignControl.value > 0 ? 0 : 1
            }
            Mixxx.ControlProxy {
                id: assignControl

                group: "[EffectRack1_EffectUnit" + (cell.index + 1) + "]"
                key: "group_" + root.groupName + "_enable"
            }
        }
    }
}
