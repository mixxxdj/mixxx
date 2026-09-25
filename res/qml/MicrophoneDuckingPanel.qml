import "." as Skin
import Mixxx 1.0 as Mixxx
import QtQuick 2.12
import "Theme"

Column {
    id: root

    enum DuckingMode {
        Off,
        Auto,
        Manual,
        NumModes // This always needs to be the last value
    }

    property string duckingModeKey: "talkoverDucking"
    property string duckingStrengthKey: "duckStrength"
    property string group: "[Master]"

    spacing: 2

    Skin.Button {
        activeColor: Theme.pflActiveButtonColor
        highlight: duckingControl.duckingEnabled
        text: duckingControl.duckingModeName
        width: 50

        onClicked: duckingControl.nextMode()
    }
    Skin.ControlKnob {
        anchors.horizontalCenter: parent.horizontalCenter
        arcStart: Knob.ArcStart.Maximum
        color: Theme.effectUnitColor
        group: root.group
        height: 35
        key: root.duckingStrengthKey
        width: 35
    }
    Mixxx.ControlProxy {
        id: duckingControl

        readonly property bool duckingEnabled: value === MicrophoneDuckingPanel.DuckingMode.Auto || value === MicrophoneDuckingPanel.DuckingMode.Manual
        readonly property string duckingModeName: {
            switch (value) {
            case MicrophoneDuckingPanel.DuckingMode.Auto:
                return "Auto";
            case MicrophoneDuckingPanel.DuckingMode.Manual:
                return "Manual";
            default:
                return "Off";
            }
        }

        function nextMode() {
            value = (value + 1) % MicrophoneDuckingPanel.DuckingMode.NumModes;
        }

        group: root.group
        key: root.duckingModeKey
    }
}
