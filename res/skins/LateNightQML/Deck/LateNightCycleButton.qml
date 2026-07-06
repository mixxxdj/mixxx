import QtQuick
import "../../../qml" as Skin
import "../LateNightTheme"

// A LateNight-styled button that cycles through N states.
LateNightIconButton {
    id: root

    required property string group
    required property string key
    property int numStates: 3
    property var stateLabels: []
    property real activeOpacity: 0.95
    property real inactiveOpacity: 0.72

    readonly property int currentState: cycleBehavior.currentState

    label: cycleBehavior.label
    labelPixelSize: 9
    labelColor: root.currentState > 0 ? LateNightTheme.textColor : LateNightTheme.textColorMuted
    contentOpacity: root.currentState > 0 ? root.activeOpacity : root.inactiveOpacity

    Skin.ControlCycleButtonBehavior {
        id: cycleBehavior

        anchors.fill: parent
        group: root.group
        key: root.key
        numberStates: root.numStates
        stateLabels: root.stateLabels
    }
}
