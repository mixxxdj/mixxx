import QtQuick
import QtQuick.Layouts
import Mixxx 1.0 as Mixxx
import "../../../qml" as Skin
import "../LateNightTheme"

Item {
    id: root

    required property string group
    property bool showRateControlButtons: true

    readonly property var deckPlayer: Mixxx.PlayerManager.getPlayer(root.group)
    readonly property bool isLoaded: deckPlayer?.isLoaded ?? false

    implicitWidth: 90
    implicitHeight: 202
    readonly property bool useSecondaryDeckText: root.group === "[Channel3]" || root.group === "[Channel4]"
    readonly property color bpmTextColor: useSecondaryDeckText ? LateNightTheme.secondaryDeckTextColor : LateNightTheme.primaryDeckTextColor
    readonly property color rateTextColor: LateNightTheme.deckReadonlyTextColor
    readonly property bool hasLegacyRateCenterAsset: LateNightTheme.optionalDeckRateCenterInactive.toString().length > 0

    function syncLeaderIconSource() {
        switch (Math.round(syncLeaderProxy.value)) {
        case 1:
            return LateNightTheme.assetDeckLeaderImplicitButton;
        case 2:
            return LateNightTheme.assetDeckLeaderExplicitButton;
        default:
            return LateNightTheme.assetDeckLeaderButton;
        }
    }

    function rateRangeTopLabelY(labelHeight) {
        return rateSlider.y - labelHeight / 2;
    }

    function rateRangeBottomLabelY(labelHeight) {
        return rateSlider.y + rateSlider.height - labelHeight / 2;
    }

    property real previousSyncEnabledValue: syncEnabledProxy.value

    Mixxx.ControlProxy {
        id: bpmProxy
        group: root.group
        key: "bpm"
    }

    Mixxx.ControlProxy {
        id: rateRatioProxy
        group: root.group
        key: "rate_ratio"
    }

    Mixxx.ControlProxy {
        id: rateDirProxy
        group: root.group
        key: "rate_dir"
    }

    Mixxx.ControlProxy {
        id: rateRangeProxy
        group: root.group
        key: "rateRange"
    }

    Mixxx.ControlProxy {
        id: rateSetDefaultProxy
        group: root.group
        key: "rate_set_default"
    }

    Mixxx.ControlProxy {
        id: syncEnabledProxy
        group: root.group
        key: "sync_enabled"
    }

    Mixxx.ControlProxy {
        id: syncLeaderProxy
        group: root.group
        key: "sync_leader"
    }

    Item {
        id: deckRateSeparator

        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: 2

        Rectangle {
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: 1
            color: LateNightTheme.deckPanelBorderDark
        }

        Rectangle {
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: 1
            color: LateNightTheme.deckPanelBorderLight
        }
    }

    ColumnLayout {
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.left: deckRateSeparator.right
        anchors.leftMargin: 2
        spacing: 3

        // BPM display
        Text {
            Layout.fillWidth: true
            Layout.preferredHeight: 24
            text: bpmProxy.value.toFixed(2)
            font.family: "Open Sans"
            font.pixelSize: 20
            font.bold: true
            color: root.bpmTextColor
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }

        // Rate percentage display
        Text {
            Layout.fillWidth: true
            Layout.preferredHeight: 17
            text: ((rateRatioProxy.value - 1) * 100).toFixed(2)
            font.family: "Open Sans"
            font.pixelSize: 13
            color: root.rateTextColor
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }

        // Sync + Leader buttons
        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: 62
            Layout.preferredHeight: 22
            spacing: 0

            // Sync button: short click performs momentary sync, hold latches sync, right-click toggles leader.
            LateNightControlButton {
                id: syncBtn
                Layout.preferredWidth: 40
                Layout.preferredHeight: 22
                backgroundSource: LateNightTheme.assetDeckSyncBackground
                activeBackgroundSuffix: "active"
                iconSource: syncEnabledProxy.value > 0 ? LateNightTheme.assetDeckSyncActiveButton : LateNightTheme.assetDeckSyncButton
                group: root.group
                key: "sync_enabled"
                rightClickKey: "sync_leader"
                longPressLatching: true
                numberStates: 2
                longPressLatchOverlayColor: LateNightTheme.syncInactiveBackgroundColor
                longPressLatchOverlayBackgroundSource: LateNightTheme.assetDeckSyncBackground
                longPressLatchOverlayIconSource: LateNightTheme.assetDeckSyncButton
                activeOpacity: 1.0
                inactiveOpacity: 1.0
                activeColor: LateNightTheme.syncExplicitLeaderColor
                fillMargin: 0
                iconLeftPadding: LateNightTheme.syncButtonHorizontalPadding
                iconRightPadding: LateNightTheme.syncButtonHorizontalPadding
            }

            // Leader button: press = request leader state; display follows sync_leader light.
            LateNightControlButton {
                id: leaderBtn
                Layout.preferredWidth: 22
                Layout.preferredHeight: 22
                backgroundSource: LateNightTheme.assetDeckLeaderBackground
                activeBackgroundSuffix: "active"
                iconSource: root.syncLeaderIconSource()
                group: root.group
                key: "sync_leader"
                displayKey: "sync_leader"
                ignoreActivePresses: true
                releaseToZero: false
                activeDisplayThreshold: 0.5
                activeOpacity: 1.0
                inactiveOpacity: 1.0
                activeColor: {
                    // sync_leader: 0=Off, 1=Soft leader, 2=Explicit leader
                    switch (Math.round(syncLeaderProxy.value)) {
                    case 1:
                        return LateNightTheme.syncImplicitLeaderColor;
                    case 2:
                        return LateNightTheme.syncExplicitLeaderColor;
                    default:
                        return "transparent";
                    }
                }
                fillMargin: 0
                onPrimaryPressed: function(displayValue) {
                    if (displayValue <= 0.5) {
                        syncBtn.startLatchReveal();
                    }
                }
            }
        }

        Connections {
            target: syncEnabledProxy

            function onValueChanged(newValue) {
                if (newValue > 0 && root.previousSyncEnabledValue <= 0) {
                    syncBtn.startLatchReveal();
                }
                root.previousSyncEnabledValue = newValue;
            }
        }

        // Rate slider + rate buttons row
        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 2

            Item {
                id: sliderContainer
                Layout.preferredWidth: 54
                Layout.fillHeight: true
                Layout.minimumHeight: 118

                // Rate range labels: top = negative direction, center = default, bottom = positive direction
                Text {
                    width: 8
                    x: 1
                    y: root.rateRangeTopLabelY(height)
                    text: "-"
                    font.family: "Open Sans"
                    font.pixelSize: 12
                    color: root.rateTextColor
                    horizontalAlignment: Text.AlignHCenter
                }

                Text {
                    width: 8
                    x: parent.width - width - 1
                    y: root.rateRangeTopLabelY(height)
                    text: (rateRangeProxy.value * 100).toFixed(0)
                    font.family: "Open Sans"
                    font.pixelSize: 12
                    color: root.rateTextColor
                    horizontalAlignment: Text.AlignRight
                }

                Text {
                    width: 8
                    x: 1
                    y: root.rateRangeBottomLabelY(height)
                    text: "+"
                    font.family: "Open Sans"
                    font.pixelSize: 12
                    color: root.rateTextColor
                    horizontalAlignment: Text.AlignHCenter
                }

                Text {
                    width: 8
                    x: parent.width - width - 1
                    y: root.rateRangeBottomLabelY(height)
                    text: (rateRangeProxy.value * 100).toFixed(0)
                    font.family: "Open Sans"
                    font.pixelSize: 12
                    color: root.rateTextColor
                    horizontalAlignment: Text.AlignRight
                }

                Skin.ControlFader {
                    id: rateSlider
                    anchors.centerIn: parent
                    width: 40
                    height: Math.min(119, parent.height - 4)
                    barColor: "#888888"
                    barMargin: 0
                    barStart: 0.5
                    bg: LateNightTheme.assetDeckRateSliderBackground
                    fg: LateNightTheme.assetDeckRateSliderHandle
                    group: root.group
                    key: "rate"

                    handleImage {
                        width: 34
                        height: 18
                        sourceSize.width: 34
                        sourceSize.height: 18
                    }
                }

                Image {
                    id: rateCenterAsset
                    width: 5
                    height: 5
                    x: rateSlider.x - 3
                    y: rateSlider.y + rateSlider.height / 2 - height / 2
                    z: rateSlider.z + 1
                    source: rateSetDefaultProxy.value > 0 ? LateNightTheme.optionalDeckRateCenterActive : LateNightTheme.optionalDeckRateCenterInactive
                    fillMode: Image.PreserveAspectFit
                    visible: root.hasLegacyRateCenterAsset
                }

                Rectangle {
                    width: 5
                    height: 5
                    x: rateSlider.x - 3
                    y: rateSlider.y + rateSlider.height / 2 - height / 2
                    z: rateSlider.z + 1
                    radius: 1
                    color: rateSetDefaultProxy.value > 0 ? "#00ffff" : "#4f4f4f"
                    border.color: LateNightTheme.deckPanelBorderDark
                    visible: !root.hasLegacyRateCenterAsset
                }
            }

            // Rate direction buttons
            ColumnLayout {
                Layout.preferredWidth: 26
                Layout.alignment: Qt.AlignVCenter
                spacing: 2
                visible: root.showRateControlButtons

                // When rate_dir = 1 (up=faster): perm_up, temp_up, temp_down, perm_down
                // When rate_dir = -1 (up=slower): perm_down, temp_down, temp_up, perm_up
                // The icon order stays the same (minus, arrow-up, arrow-down, plus),
                // but the bound keys swap to match the rate direction.

                LateNightControlButton {
                    Layout.preferredWidth: 26
                    Layout.preferredHeight: 26
                    backgroundSource: LateNightTheme.lateNightTopRegionButton("square")
                    iconSource: LateNightTheme.assetDeckMinusButton
                    group: root.group
                    key: rateDirProxy.value >= 0 ? "rate_perm_up" : "rate_perm_down"
                    rightClickKey: rateDirProxy.value >= 0 ? "rate_perm_up_small" : "rate_perm_down_small"
                    activeOpacity: 0.95
                    inactiveOpacity: 0.76
                    inactiveColor: LateNightTheme.deckEmbeddedButtonInactiveColor
                }

                LateNightControlButton {
                    Layout.preferredWidth: 26
                    Layout.preferredHeight: 26
                    backgroundSource: LateNightTheme.lateNightTopRegionButton("square")
                    iconSource: LateNightTheme.assetDeckArrowLeftUpButton
                    group: root.group
                    key: rateDirProxy.value >= 0 ? "rate_temp_up" : "rate_temp_down"
                    rightClickKey: rateDirProxy.value >= 0 ? "rate_temp_up_small" : "rate_temp_down_small"
                    activeOpacity: 0.95
                    inactiveOpacity: 0.76
                    inactiveColor: LateNightTheme.deckEmbeddedButtonInactiveColor
                }

                LateNightControlButton {
                    Layout.preferredWidth: 26
                    Layout.preferredHeight: 26
                    backgroundSource: LateNightTheme.lateNightTopRegionButton("square")
                    iconSource: LateNightTheme.assetDeckArrowRightDownButton
                    group: root.group
                    key: rateDirProxy.value >= 0 ? "rate_temp_down" : "rate_temp_up"
                    rightClickKey: rateDirProxy.value >= 0 ? "rate_temp_down_small" : "rate_temp_up_small"
                    activeOpacity: 0.95
                    inactiveOpacity: 0.76
                    inactiveColor: LateNightTheme.deckEmbeddedButtonInactiveColor
                }

                LateNightControlButton {
                    Layout.preferredWidth: 26
                    Layout.preferredHeight: 26
                    backgroundSource: LateNightTheme.lateNightTopRegionButton("square")
                    iconSource: LateNightTheme.assetDeckPlusButton
                    group: root.group
                    key: rateDirProxy.value >= 0 ? "rate_perm_down" : "rate_perm_up"
                    rightClickKey: rateDirProxy.value >= 0 ? "rate_perm_down_small" : "rate_perm_up_small"
                    activeOpacity: 0.95
                    inactiveOpacity: 0.76
                    inactiveColor: LateNightTheme.deckEmbeddedButtonInactiveColor
                }
            }
        }
    }
}
