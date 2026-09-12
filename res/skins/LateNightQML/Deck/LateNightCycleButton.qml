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
    property color activeLabelColor: LateNightTheme.textColor
    property color inactiveLabelColor: LateNightTheme.textColorMuted
    property bool activeWhenNonzero: false

    readonly property int currentState: cycleBehavior.currentState

    activeState: root.activeWhenNonzero && root.currentState > 0
    label: cycleBehavior.label
    // Legacy LateNight applies the common 11px WPushButton font to vinyl
    // mode/cue buttons; the old 9px override made these labels visibly
    // smaller than VINYL and PASS.
    labelPixelSize: 11
    labelColor: root.currentState > 0 ? root.activeLabelColor : root.inactiveLabelColor
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
