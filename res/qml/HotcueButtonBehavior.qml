import "." as Skin
import Mixxx 1.0 as Mixxx
import QtQuick 2.12

Skin.ControlProxyButtonBehavior {
    id: root

    required property int hotcueNumber
    property color fallbackColor: "white"
    readonly property color hotcueColor: colorProxy.value >= 0
            ? "#" + Math.round(colorProxy.value).toString(16).padStart(6, "0")
            : root.fallbackColor
    readonly property bool isSet: statusProxy.value > 0

    signal popupRequested(real mouseX, real mouseY)
    signal cleared()

    key: "hotcue_" + root.hotcueNumber + "_activate"
    displayKey: "hotcue_" + root.hotcueNumber + "_status"

    Mixxx.ControlProxy {
        id: colorProxy

        group: root.group
        key: "hotcue_" + root.hotcueNumber + "_color"
    }

    Mixxx.ControlProxy {
        id: statusProxy

        group: root.group
        key: "hotcue_" + root.hotcueNumber + "_status"

        onValueChanged: function(newValue) {
            if (newValue === 0) {
                root.cleared();
            }
        }
    }

    onSecondaryPressed: function(displayValue, mouseX, mouseY) {
        if (displayValue > 0) {
            root.popupRequested(mouseX, mouseY);
        }
    }
}
