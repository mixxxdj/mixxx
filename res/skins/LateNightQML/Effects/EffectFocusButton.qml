import Mixxx 1.0 as Mixxx
import QtQuick
import "../LateNightTheme"

Item {
    id: root

    required property int effectNumber
    readonly property bool focused: Math.round(focusedEffect.value) === effectNumber
    required property string unitGroup

    Image {
        anchors.fill: parent
        source: root.focused ? LateNightTheme.assetFxFocusActiveButton : LateNightTheme.assetFxFocusButton
    }
    MouseArea {
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        anchors.fill: parent

        onClicked: mouse => {
            if (mouse.button === Qt.RightButton) {
                focusedEffect.value = 0;
            } else {
                focusedEffect.value = root.focused ? 0 : root.effectNumber;
            }
        }
    }
    Mixxx.ControlProxy {
        id: focusedEffect

        group: root.unitGroup
        key: "focused_effect"
    }
}
