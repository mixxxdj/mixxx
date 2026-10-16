import Mixxx 1.0 as Mixxx
import QtQuick 2.12

Item {
    id: root

    required property string group
    required property string key
    property string decrementKey: ""
    property string incrementKey: ""
    property var beatSizes: [
        1 / 32,
        1 / 16,
        1 / 8,
        1 / 4,
        1 / 2,
        1,
        2,
        4,
        8,
        16,
        32,
        64
    ]
    readonly property bool useStepControls: decrementKey.length > 0 && incrementKey.length > 0
    readonly property real value: valueProxy.value
    readonly property string valueText: formatBeatSize(valueProxy.value)

    signal committed(real value)
    signal stepped(int delta)

    function formatBeatSize(value) {
        if (value <= 0) {
            return "";
        }
        if (value < 1) {
            return "1/" + Math.round(1 / value);
        }
        if (Math.abs(value - Math.round(value)) < 0.0001) {
            return Math.round(value).toString();
        }
        return value.toString();
    }

    function parseBeatSize(text) {
        const parts = text.split("/");
        if (parts.length === 2) {
            const numerator = Number(parts[0]);
            const denominator = Number(parts[1]);
            if (numerator > 0 && denominator > 0) {
                return numerator / denominator;
            }
            return valueProxy.value;
        }
        const parsed = Number(text);
        return parsed > 0 ? parsed : valueProxy.value;
    }

    function nearestBeatSizeIndex(value) {
        let nearestIndex = 0;
        let nearestDistance = Math.abs(root.beatSizes[0] - value);
        for (let i = 1; i < root.beatSizes.length; ++i) {
            const distance = Math.abs(root.beatSizes[i] - value);
            if (distance < nearestDistance) {
                nearestDistance = distance;
                nearestIndex = i;
            }
        }
        return nearestIndex;
    }

    function step(delta) {
        if (root.useStepControls) {
            if (delta < 0) {
                decrementControl.trigger();
            } else {
                incrementControl.trigger();
            }
            root.stepped(delta);
            return;
        }

        let index = nearestBeatSizeIndex(valueProxy.value);
        index = Math.max(0, Math.min(root.beatSizes.length - 1, index + delta));
        valueProxy.value = root.beatSizes[index];
        root.stepped(delta);
    }

    function commitText(text) {
        valueProxy.value = parseBeatSize(text);
        root.committed(valueProxy.value);
        return root.valueText;
    }

    Mixxx.ControlProxy {
        id: valueProxy

        group: root.group
        key: root.key
    }

    Mixxx.ControlProxy {
        id: decrementControl

        group: root.group
        key: root.decrementKey.length > 0 ? root.decrementKey : root.key
    }

    Mixxx.ControlProxy {
        id: incrementControl

        group: root.group
        key: root.incrementKey.length > 0 ? root.incrementKey : root.key
    }
}
