import QtQuick
import Mixxx 1.0 as Mixxx
import "../LateNightTheme"

Item {
    id: root

    required property string group

    implicitWidth: 111
    implicitHeight: 20

    Mixxx.ControlProxy {
        id: keyProxy
        group: root.group
        key: "key"
    }

    Mixxx.ControlProxy {
        id: keyNotationProxy
        group: "[Library]"
        key: "key_notation"
    }

    Mixxx.ControlProxy {
        id: visualKeyDistanceProxy
        group: root.group
        key: "visual_key_distance"
    }

    function keyToOpenKeyNumber(key) {
        return Mixxx.KeyUtils.keyToOpenKeyNumber(key);
    }

    function scaledKey(key, steps) {
        return Mixxx.KeyUtils.scaleKeySteps(key, steps);
    }

    function colorForKey(key) {
        const openKeyNumber = keyToOpenKeyNumber(key);
        const palette = Mixxx.Config.keyColorPalette;
        if (openKeyNumber <= 0 || !palette || palette.length < openKeyNumber) {
            return LateNightTheme.accentColor;
        }
        return palette[openKeyNumber - 1];
    }

    function keyDisplayText(key, notationRevision) {
        notationRevision;
        return Mixxx.KeyUtils.keyToString(key);
    }

    readonly property real keyDistance: visualKeyDistanceProxy.value
    readonly property real splitPoint: Math.max(0, Math.min(1, keyDistance < 0 ? keyDistance + 1 : keyDistance))
    readonly property color stripTopColor: keyDistance < 0 ? colorForKey(keyProxy.value) : colorForKey(scaledKey(keyProxy.value, 1))
    readonly property color stripBottomColor: keyDistance < 0 ? colorForKey(scaledKey(keyProxy.value, -1)) : colorForKey(keyProxy.value)
    readonly property string displayKeyText: keyDisplayText(keyProxy.value, keyNotationProxy.value)
    readonly property bool useSecondaryDeckText: root.group === "[Channel3]" || root.group === "[Channel4]"
    readonly property color keyTextColor: useSecondaryDeckText ? LateNightTheme.secondaryDeckTextColor : LateNightTheme.primaryDeckTextColor

    // Key match/reset button: left-click = sync_key, right-click = reset_key
    LateNightControlButton {
        x: 0
        y: 0
        width: 26
        height: 20
        backgroundSource: LateNightTheme.assetDeckKeyButtonBackground
        iconSource: LateNightTheme.assetDeckKeyMatchButton
        iconLeftPadding: 5
        iconRightPadding: 5
        iconTopPadding: 3
        iconBottomPadding: 3
        group: root.group
        key: "sync_key"
        rightClickKey: "reset_key"
        activeOpacity: 1.0
        inactiveOpacity: 0.78
        inactiveColor: LateNightTheme.deckEmbeddedButtonInactiveColor
        pressedBackgroundSuffix: "active"
        pressedIconSuffix: LateNightTheme.keyControlsPressedIconSuffix
        pressedActivatesFill: true
        activeColor: LateNightTheme.keyControlsPressedColor
        useBorderImageBackground: true
        backgroundBorderTop: 2
        backgroundBorderBottom: 2
        backgroundBorderLeft: 2
        backgroundBorderRight: 2
    }

    // Visual key distance strip
    Rectangle {
        x: 26
        y: 0
        width: 4
        height: 20

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

    // Key text display
    Rectangle {
        x: 30
        y: 0
        width: root.width - 50
        height: 20
        color: "#181818"

        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            height: 1
            color: LateNightTheme.deckPanelBorderLight
        }

        Rectangle {
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: 1
            color: LateNightTheme.deckPanelBorderLight
        }

        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            height: 1
            color: LateNightTheme.deckPanelBorderDark
        }

        Rectangle {
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: 1
            color: LateNightTheme.deckPanelBorderDark
        }

        Text {
            anchors.fill: parent
            anchors.leftMargin: 6
            anchors.rightMargin: 6
            text: root.displayKeyText.length > 0 ? root.displayKeyText : "--"
            font.family: "Open Sans"
            font.pixelSize: 12
            font.weight: Font.Medium
            color: root.keyTextColor
            elide: Text.ElideRight
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
    }

    // Key up/down buttons
    Column {
        x: root.width - 20
        y: 0
        width: 20
        height: 20
        spacing: 0

        // Key Up: left-click = pitch_up, right-click = pitch_up_small
        LateNightControlButton {
            width: 20
            height: 10
            backgroundSource: LateNightTheme.assetDeckKeyButtonBackground
            iconSource: LateNightTheme.assetDeckKeyUpButton
            iconLeftPadding: 4
            iconRightPadding: 4
            iconTopPadding: 1
            iconBottomPadding: 1
            group: root.group
            key: "pitch_up"
            rightClickKey: "pitch_up_small"
            activeOpacity: 0.95
            inactiveOpacity: 0.72
            inactiveColor: LateNightTheme.deckEmbeddedButtonInactiveColor
            pressedBackgroundSuffix: "active"
            pressedIconSuffix: LateNightTheme.keyControlsPressedIconSuffix
            pressedActivatesFill: true
            activeColor: LateNightTheme.keyControlsPressedColor
            useBorderImageBackground: true
            backgroundBorderTop: 1
            backgroundBorderBottom: 12
            backgroundBorderLeft: 2
            backgroundBorderRight: 2
        }

        // Key Down: left-click = pitch_down, right-click = pitch_down_small
        LateNightControlButton {
            width: 20
            height: 10
            backgroundSource: LateNightTheme.assetDeckKeyButtonBackground
            iconSource: LateNightTheme.assetDeckKeyDownButton
            iconLeftPadding: 4
            iconRightPadding: 4
            iconTopPadding: 1
            iconBottomPadding: 1
            group: root.group
            key: "pitch_down"
            rightClickKey: "pitch_down_small"
            activeOpacity: 0.95
            inactiveOpacity: 0.72
            inactiveColor: LateNightTheme.deckEmbeddedButtonInactiveColor
            pressedBackgroundSuffix: "active"
            pressedIconSuffix: LateNightTheme.keyControlsPressedIconSuffix
            pressedActivatesFill: true
            activeColor: LateNightTheme.keyControlsPressedColor
            useBorderImageBackground: true
            backgroundBorderTop: 12
            backgroundBorderBottom: 1
            backgroundBorderLeft: 2
            backgroundBorderRight: 2
        }
    }
}
