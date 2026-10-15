import QtQuick
import Mixxx 1.0 as Mixxx
import "../LateNightTheme"

Item {
    id: root

    required property string group
    property url backgroundSource: LateNightTheme.lateNightSubRegionButton("play")
    property string playIconName: "play"
    property string pauseIconName: "pause"
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
        activeIconSuffix: root.useFullIcon && LateNightTheme.isPaleMoon ? "active" : ""
        activeOpacity: 1.0
        activeColor: LateNightTheme.activePlayCueColor
        backgroundSource: root.backgroundSource
        displayKey: "play_latched"
        group: root.group
        iconName: root.useFullIcon
                ? "circle-play"
                : (button.isActive ? root.pauseIconName : root.playIconName)
        inactiveOpacity: 0.82
        key: "play"
        releaseToZero: false
        rightClickKey: root.rightClickKey
        toggleable: true
        visualActiveState: indicatorProxy.value > 0
    }

    Rectangle {
        anchors.fill: parent
        anchors.margins: 1
        border.color: LateNightTheme.activePlayCueColor
        border.width: 3
        color: "transparent"
        visible: previewProxy.value > 0 && !button.isActive
    }
}
