import Mixxx 1.0 as Mixxx
import QtQuick
import "../LateNightTheme"

Item {
    id: root

    readonly property bool active: control.value > 0
    property url activeBackgroundSource: backgroundSource
    property color activeColor: LateNightTheme.effectsControlActiveColor
    property url activeSource: normalSource
    property int backgroundInset: 2
    property url backgroundSource
    required property string group
    required property string key
    property color normalColor: LateNightTheme.effectsControlInactiveColor
    property url normalSource
    property bool toggleable: true

    Rectangle {
        anchors.fill: parent
        anchors.margins: root.backgroundInset
        color: root.active ? root.activeColor : root.normalColor
    }
    Image {
        anchors.fill: parent
        fillMode: Image.Stretch
        source: root.active ? root.activeBackgroundSource : root.backgroundSource
    }
    Image {
        anchors.fill: parent
        fillMode: Image.Stretch
        source: root.active ? root.activeSource : root.normalSource
    }
    TapHandler {
        onTapped: {
            if (root.toggleable) {
                control.value = root.active ? 0 : 1;
            } else {
                control.value = 1;
            }
        }
    }
    Mixxx.ControlProxy {
        id: control

        group: root.group
        key: root.key
    }
}
