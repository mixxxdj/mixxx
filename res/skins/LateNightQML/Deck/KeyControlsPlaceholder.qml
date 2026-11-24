import QtQuick
import Mixxx 1.0 as Mixxx
import "../LateNightTheme"
import "../../../qml/Mixxx/Controls" as MixxxControls

Item {
    id: root

    required property string group

    // Crown (22) + Sync (40) + Match Key (26) + key text + up/down.
    implicitWidth: 138
    implicitHeight: 22

    Mixxx.ControlProxy {
        id: keyProxy
        group: root.group
        key: "key"
    }

    Mixxx.ControlProxy {
        id: syncKeyProxy
        group: root.group
        key: "sync_key"
    }

    Mixxx.ControlProxy {
        id: keyLeaderProxy
        group: root.group
        key: "sync_key_leader"
    }

    function keyLeaderIcon() {
        switch (Math.round(keyLeaderProxy.value)) {
        case 1:
            return LateNightTheme.assetDeckLeaderImplicitButton;
        case 2:
            return LateNightTheme.assetDeckLeaderExplicitButton;
        default:
            return LateNightTheme.assetDeckLeaderButton;
        }
    }

    function keyLeaderColor() {
        switch (Math.round(keyLeaderProxy.value)) {
        case 1:
            return LateNightTheme.syncImplicitLeaderColor;
        case 2:
            return LateNightTheme.syncExplicitLeaderColor;
        default:
            return "transparent";
        }
    }

    Mixxx.ControlProxy {
        id: trackLoadedProxy
        group: root.group
        key: "track_loaded"
    }

    Mixxx.ControlProxy {
        id: keyNotationProxy
        group: "[Library]"
        key: "key_notation"
    }

    function keyDisplayText(key, notationRevision) {
        return Mixxx.KeyUtils.keyToString(key, notationRevision);
    }

    readonly property string displayKeyText: keyDisplayText(keyProxy.value, keyNotationProxy.value)
    readonly property bool showKeyColorStrip: Mixxx.Config.configKeyColorsEnabled && trackLoadedProxy.value > 0 && displayKeyText.length > 0
    readonly property int keyTextX: showKeyColorStrip ? 92 : 88
    readonly property bool useSecondaryDeckText: root.group === "[Channel3]" || root.group === "[Channel4]"
    readonly property color keyTextColor: useSecondaryDeckText ? LateNightTheme.secondaryDeckTextColor : LateNightTheme.primaryDeckTextColor

    // Crown, then Sync, then Match Key. cycleStates writes 0/1/2 itself
    // because the key crown is a plain 3-state control. The engine folds 2
    // back to 1, the same as the beat crown. A lit crown latches key sync.
    // Turning sync off removes the crown.
    LateNightControlButton {
        id: keyLeaderButton
        z: 2
        x: 0
        y: 0
        width: 22
        height: 22
        backgroundSource: LateNightTheme.assetDeckSyncBackground
        activeBackgroundSuffix: "active"
        iconSource: root.keyLeaderIcon()
        group: root.group
        key: "sync_key_leader"
        displayKey: "sync_key_leader"
        numberStates: 3
        cycleStates: true
        releaseToZero: false
        activeOpacity: 1.0
        inactiveOpacity: 1.0
        activeColor: root.keyLeaderColor()
        fillMargin: 0
    }

    // First press with no key leader lights the crown and does not retune.
    // A later press matches the leader. Turning sync off removes the crown.
    // Right-click hold activates sync the same way. While sync is on,
    // right-click does nothing. Reset stays on the match button.
    LateNightControlButton {
        id: keySyncButton
        z: 2
        x: 22
        y: 0
        width: 40
        height: 22
        backgroundSource: LateNightTheme.assetDeckSyncBackground
        activeBackgroundSuffix: "active"
        iconSource: syncKeyProxy.value > 0 ? LateNightTheme.assetDeckSyncActiveButton : LateNightTheme.assetDeckSyncButton
        group: root.group
        key: "sync_key"
        rightClickMatchesLeft: true
        longPressLatching: true
        numberStates: 2
        longPressLatchOverlayColor: LateNightTheme.syncInactiveBackgroundColor
        longPressLatchOverlayBackgroundSource: LateNightTheme.assetDeckSyncBackground
        longPressLatchOverlayIconSource: LateNightTheme.assetDeckSyncButton
        activeOpacity: 1.0
        inactiveOpacity: 1.0
        activeColor: LateNightTheme.syncExplicitLeaderColor
        fillMargin: 0
        iconBottomPadding: LateNightTheme.isPaleMoon ? 2 : 0
        iconLeftPadding: LateNightTheme.isPaleMoon ? 2 : LateNightTheme.syncButtonHorizontalPadding
        iconRightPadding: LateNightTheme.isPaleMoon ? 2 : LateNightTheme.syncButtonHorizontalPadding
        iconTopPadding: LateNightTheme.isPaleMoon ? 2 : 0
        rasterizeIconAtPaintedSize: LateNightTheme.isPaleMoon
        stretchIcon: LateNightTheme.isPaleMoon
    }

    // Key match/reset button: tap matches once and does not light Sync.
    // Right-click resets the key. The leader keeps sync and other synced
    // decks follow. Any other deck turns key sync off.
    // Hold-to-latch is the Sync button.
    LateNightControlButton {
        x: 62
        y: 0
        width: 26
        height: 22
        backgroundSource: LateNightTheme.assetDeckKeyButtonBackground
        iconSource: LateNightTheme.assetDeckKeyMatchButton
        iconLeftPadding: 5
        iconRightPadding: 5
        iconTopPadding: 3
        iconBottomPadding: 3
        group: root.group
        key: "match_key"
        rightClickKey: "reset_key"
        longPressLatching: false
        numberStates: 2
        activeOpacity: 1.0
        inactiveOpacity: 0.78
        inactiveColor: LateNightTheme.deckEmbeddedButtonInactiveColor
        pressedBackgroundSuffix: "active"
        pressedIconSuffix: LateNightTheme.keyControlsPressedIconSuffix
        pressedActivatesFill: true
        pressedColor: LateNightTheme.keyControlsPressedColor
        activeColor: "transparent"
        useBorderImageBackground: true
        backgroundBorderTop: 2
        backgroundBorderBottom: 2
        backgroundBorderLeft: 2
        backgroundBorderRight: 2
    }

    MixxxControls.KeyColorIndicator {
        fallbackColor: LateNightTheme.accentColor
        x: 88
        y: 1
        width: 4
        height: 20
        group: root.group
        visible: root.showKeyColorStrip
    }

    // Key text display
    Rectangle {
        x: root.keyTextX
        y: 1
        width: root.width - root.keyTextX - 20
        height: 20
        color: LateNightTheme.deckTopRowBackgroundColor

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
            text: root.displayKeyText
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
        y: 1
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
