import Mixxx 1.0 as Mixxx
import QtQuick 2.12

Item {
    id: root

    required property string group
    required property string key
    property int numberStates: 3
    property var stateLabels: []
    property bool handlePointerInput: true
    readonly property int currentState: Math.round(control.value)
    readonly property real displayValue: control.value
    readonly property string label: {
        if (root.stateLabels.length > root.currentState && root.currentState >= 0) {
            return root.stateLabels[root.currentState];
        }
        return root.currentState.toString();
    }

    signal cycled(int state)

    function cycle() {
        const newState = (root.currentState + 1) % root.numberStates;
        control.value = newState;
        root.cycled(newState);
    }

    Mixxx.ControlProxy {
        id: control

        group: root.group
        key: root.key
    }

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton
        enabled: root.enabled && root.handlePointerInput

        onClicked: root.cycle()
    }
}
