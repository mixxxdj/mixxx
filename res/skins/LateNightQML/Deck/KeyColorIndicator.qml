import QtQuick
import Mixxx 1.0 as Mixxx
import "../LateNightTheme"

Item {
    id: root

    required property string group

    implicitWidth: 4
    implicitHeight: 20

    Mixxx.ControlProxy {
        id: keyProxy

        group: root.group
        key: "key"
    }

    Mixxx.ControlProxy {
        id: visualKeyDistanceProxy

        group: root.group
        key: "visual_key_distance"
    }

    function colorForKey(key) {
        const openKeyNumber = Mixxx.KeyUtils.keyToOpenKeyNumber(key);
        const palette = Mixxx.Config.keyColorPalette;
        if (openKeyNumber <= 0 || !palette || palette.length < openKeyNumber) {
            return LateNightTheme.accentColor;
        }
        return palette[openKeyNumber - 1];
    }

    readonly property real keyDistance: visualKeyDistanceProxy.value
    readonly property real splitPoint: Math.max(0, Math.min(1, keyDistance < 0 ? keyDistance + 1 : keyDistance))
    readonly property color stripTopColor: keyDistance < 0 ? colorForKey(keyProxy.value) : colorForKey(Mixxx.KeyUtils.scaleKeySteps(keyProxy.value, 1))
    readonly property color stripBottomColor: keyDistance < 0 ? colorForKey(Mixxx.KeyUtils.scaleKeySteps(keyProxy.value, -1)) : colorForKey(keyProxy.value)

    Rectangle {
        anchors.fill: parent

        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            height: parent.height * root.splitPoint
            color: root.stripTopColor
        }

        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            height: parent.height - parent.height * root.splitPoint
            color: root.stripBottomColor
        }
    }
}
