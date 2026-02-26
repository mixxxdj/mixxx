import "." as Skin
import Mixxx 1.0 as Mixxx
import QtQuick 2.12
import "Theme"

Column {
    enum DuckingMode {
        Off,
        Auto,
        Manual,
        NumModes // This always needs to be the last value
    }

    Skin.ControlFader {
        barColor: Theme.crossfaderBarColor
        barStart: 1
        bg: Theme.imgMicDuckingSlider
        fg: Theme.imgMicDuckingSliderHandle
        group: "[Master]"
        height: 26
        key: "duckStrength"
        orientation: Qt.Horizontal
        width: 50
    }
    Skin.Button {
        id: pflButton

        activeColor: Theme.pflActiveButtonColor
        highlight: duckingControl.duckingEnabled
        text: duckingControl.duckingModeName

        onClicked: duckingControl.nextMode()

        Mixxx.ControlProxy {
            id: duckingControl

            property bool duckingEnabled: {
                return (this.value == MicrophoneDuckingPanel.DuckingMode.Auto || this.value == MicrophoneDuckingPanel.DuckingMode.Manual);
            }
            property string duckingModeName: {
                switch (this.value) {
                case MicrophoneDuckingPanel.DuckingMode.Auto:
                    return "Auto";
                case MicrophoneDuckingPanel.DuckingMode.Manual:
                    return "Manual";
                default:
                    return "Off";
                }
            }

            function nextMode() {
                this.value = (this.value + 1) % MicrophoneDuckingPanel.DuckingMode.NumModes;
            }

            group: "[Master]"
            key: "talkoverDucking"
        }
    }
}
