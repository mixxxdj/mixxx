import QtQuick
import Mixxx 1.0 as Mixxx
import "../LateNightTheme"

Item {
    id: root

    required property string group
    property url backgroundSource: LateNightTheme.lateNightSubRegionButton("play")
    property url playIcon: LateNightTheme.assetDeckPlayMiniButton
    property url pauseIcon: LateNightTheme.assetDeckPauseMiniButton
    property string rightClickKey: "cue_set"
    property bool useFullIcon: false
    readonly property bool toggledTransport: button.toggleable && !button.releaseToZero
    readonly property string displayControlKey: button.displayKey
    readonly property string rightClickControlKey: button.rightClickKey

    implicitWidth: 68
    implicitHeight: 26

    Mixxx.ControlProxy {
        id: previewProxy

        group: root.group
        key: "play"
    }

    Mixxx.ControlProxy {
        id: indicatorProxy

        group: root.group
        key: "play_indicator"
    }

    LateNightControlButton {
        id: button

        anchors.fill: parent
        activeBackgroundSuffix: "active"
        activeColor: LateNightTheme.activePlayCueColor
        activeOpacity: 1.0
        backgroundSource: root.backgroundSource
        displayKey: "play_latched"
        group: root.group
        // The legacy full-deck asset is a play/pause sprite. Rendering that
        // sprite leaves both glyphs visible, so always select one state-specific
        // glyph for stopped/playing transport states.
        iconSource: button.isActive ? root.pauseIcon : root.playIcon
        inactiveOpacity: 0.82
        key: "play"
        releaseToZero: false
        rightClickKey: root.rightClickKey
        toggleable: true
    }

    Rectangle {
        anchors.fill: parent
        border.color: LateNightTheme.activePlayCueColor
        border.width: 3
        color: "transparent"
        visible: previewProxy.value > 0
    }

    Rectangle {
        anchors.fill: parent
        border.color: LateNightTheme.activePlayCueColor
        border.width: 1
        color: "transparent"
        opacity: indicatorProxy.value > 0 ? 1 : 0
        visible: indicatorProxy.value > 0
    }
}
