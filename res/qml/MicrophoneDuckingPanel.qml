import "." as Skin
import Mixxx 0.1 as Mixxx
import QtQuick 2.12
import "Theme"

Column {

    enum DuckingMode {
        Off,
        Auto,
        Manual,
        NumModes // This always needs to be the last value
    }

    Skin.ControlSlider {
        width: 50
        height: 26
        orientation: Qt.Horizontal
        group: "[Master]"
        key: "duckStrength"
        barColor: Theme.crossfaderBarColor
        barStart: 1
        fg: Theme.imgMicDuckingSliderHandle
        bg: Theme.imgMicDuckingSlider
    }

    Skin.Button {
        id: pflButton

        text: duckingControl.duckingModeName
        activeColor: Theme.pflActiveButtonColor
        highlight: duckingControl.duckingEnabled
        onClicked: duckingControl.nextMode()

        Mixxx.ControlProxy {
            id: duckingControl

            property string duckingModeName: {
                switch (value) {
                case MicrophoneDuckingPanel.DuckingMode.Auto:
                    return "Auto";
                case MicrophoneDuckingPanel.DuckingMode.Manual:
                    return "Manual";
                default:
                    return "Off";
                }
            }
            property bool duckingEnabled: {
                return (value == MicrophoneDuckingPanel.DuckingMode.Auto || value == MicrophoneDuckingPanel.DuckingMode.Manual);
            }

            function nextMode() {
                value = (value + 1) % MicrophoneDuckingPanel.DuckingMode.NumModes;
            }

            group: "[Master]"
            key: "talkoverDucking"
        }

    }

}
