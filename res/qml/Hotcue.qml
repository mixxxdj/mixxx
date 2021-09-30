import Mixxx 0.1 as Mixxx
import QtQuick 2.12
import "Theme"

Item {
    id: root

    property int hotcueNumber // required
    property string group // required
    property alias activate: hotcueActivateControl.value
    property alias clear: hotcueClearControl.value
    readonly property bool isSet: hotcueStatusControl.value != 0
    readonly property color color: {
        if (hotcueColorControl.value < 0)
            return Theme.deckActiveColor;

        return "#" + hotcueColorControl.value.toString(16).padStart(6, "0");
    }

    function setColor(newColor) {
        hotcueColorControl.value = (parseInt(newColor.r * 255) << 16) | (parseInt(newColor.g * 255) << 8) | parseInt(newColor.b * 255);
    }

    Mixxx.ControlProxy {
        id: hotcueColorControl

        group: root.group
        key: "hotcue_" + hotcueNumber + "_color"
    }

    Mixxx.ControlProxy {
        id: hotcueActivateControl

        group: root.group
        key: "hotcue_" + hotcueNumber + "_activate"
    }

    Mixxx.ControlProxy {
        id: hotcueStatusControl

        group: root.group
        key: "hotcue_" + hotcueNumber + "_status"
    }

    Mixxx.ControlProxy {
        id: hotcueClearControl

        group: root.group
        key: "hotcue_" + hotcueNumber + "_clear"
    }

}
