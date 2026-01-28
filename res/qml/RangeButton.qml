import "." as Skin
import Mixxx 1.0 as Mixxx
import "Theme"

Skin.Button {
    id: root

    required property string group
    property var modes: [0.04, 0.06, 0.08, 0.10, 0.16, 0.24, 0.50, 1]
    property alias range: rangeControl.value

    function nextRange() {
        const currentRange = rangeControl.value;
        let currentIdx = 0;
        let currentDiff = Math.abs(currentRange - root.modes[0]);
        for (let i = 1; i < root.modes.length; i++) {
            let diff = Math.abs(currentRange - root.modes[i]);
            if (currentDiff > diff) {
                currentIdx = i;
                currentDiff = diff;
            }
        }
        rangeControl.value = root.modes[(currentIdx + 1) % root.modes.length];
    }

    text: "Range"

    onClicked: nextRange()

    Mixxx.ControlProxy {
        id: rangeControl

        group: root.group
        key: "rateRange"
    }
}
