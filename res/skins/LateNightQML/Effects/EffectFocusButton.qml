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
    TapHandler {
        acceptedButtons: Qt.LeftButton

        onTapped: focusedEffect.value = root.focused ? 0 : root.effectNumber
    }
    TapHandler {
        acceptedButtons: Qt.RightButton

        onTapped: focusedEffect.value = 0
    }
    Mixxx.ControlProxy {
        id: focusedEffect

        group: root.unitGroup
        key: "focused_effect"
    }
}
