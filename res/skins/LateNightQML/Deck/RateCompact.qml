import QtQuick
import QtQuick.Layouts
import Mixxx 1.0 as Mixxx
import "../../../qml" as Skin
import "../LateNightTheme"

Item {
    id: root

    readonly property color displayColor: secondaryDeck ? LateNightTheme.secondaryDeckTextColor : LateNightTheme.primaryDeckTextColor
    required property string group
    readonly property bool secondaryDeck: root.group === "[Channel3]" || root.group === "[Channel4]"
    readonly property bool showSyncButton: showSyncButtonProxy.value > 0
    readonly property int sliderHeight: showSyncButton ? 79 : 95

    function leaderIcon() {
        switch (Math.round(syncLeaderProxy.value)) {
        case 1:
            return LateNightTheme.assetDeckLeaderImplicitButton;
        case 2:
            return LateNightTheme.assetDeckLeaderExplicitButton;
        default:
            return LateNightTheme.assetDeckLeaderButton;
        }
    }

    implicitHeight: 168
    implicitWidth: 62

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
    Mixxx.ControlProxy {
        id: showSyncButtonProxy

        group: "[Skin]"
        key: "latenight_show_sync_button_compact"
    }
    Rectangle {
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.top: parent.top
        color: LateNightTheme.deckPanelBorderDark
        width: 2
    }
    ColumnLayout {
        anchors.fill: parent
        anchors.leftMargin: 3
        spacing: 0

        ColumnLayout {
            Layout.fillWidth: true
            Layout.preferredHeight: 42
            spacing: 0

            Text {
                Layout.fillWidth: true
                Layout.preferredHeight: 24
                color: root.displayColor
                font.bold: true
                font.family: "Open Sans"
                font.pixelSize: 18
                horizontalAlignment: Text.AlignHCenter
                text: bpmProxy.value.toFixed(2)
                verticalAlignment: Text.AlignVCenter
            }
            Text {
                Layout.fillWidth: true
                Layout.preferredHeight: 18
                color: root.displayColor
                font.family: "Open Sans"
                font.pixelSize: 12
                horizontalAlignment: Text.AlignHCenter
                text: ((rateRatioProxy.value - 1) * 100).toFixed(2)
                verticalAlignment: Text.AlignVCenter
            }
            TapHandler {
                onTapped: {
                    bpmTapProxy.value = 1.0;
                    bpmTapProxy.value = 0.0;
                }
            }
        }
        Mixxx.ControlProxy {
            id: bpmTapProxy

            group: root.group
            key: "bpm_tap"
        }
        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredHeight: root.showSyncButton ? 22 : 0
            Layout.preferredWidth: 62
            spacing: 0
            visible: root.showSyncButton

            LateNightControlButton {
                Layout.preferredHeight: 22
                Layout.preferredWidth: 40
                activeColor: LateNightTheme.syncExplicitLeaderColor
                activeOpacity: 1.0
                backgroundSource: LateNightTheme.assetDeckSyncBackground
                fillMargin: 0
                group: root.group
                iconSource: syncEnabledProxy.value > 0 ? LateNightTheme.assetDeckSyncActiveButton : LateNightTheme.assetDeckSyncButton
                inactiveOpacity: 1.0
                key: "sync_enabled"
                longPressLatching: true
                numberStates: 2
                rightClickKey: "sync_leader"
            }
            LateNightControlButton {
                Layout.preferredHeight: 22
                Layout.preferredWidth: 22
                activeOpacity: 1.0
                backgroundSource: LateNightTheme.assetDeckLeaderBackground
                displayKey: "sync_leader"
                fillMargin: 0
                group: root.group
                iconSource: root.leaderIcon()
                ignoreActivePresses: true
                inactiveOpacity: 1.0
                key: "sync_leader"
                releaseToZero: false
            }
        }
        Item {
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredHeight: root.sliderHeight + 24
            Layout.preferredWidth: 58

            Text {
                anchors.left: parent.left
                anchors.top: parent.top
                color: root.displayColor
                font.pixelSize: 11
                horizontalAlignment: Text.AlignHCenter
                text: "−"
                width: 10
            }
            Text {
                anchors.right: parent.right
                anchors.top: parent.top
                color: root.displayColor
                font.pixelSize: 10
                horizontalAlignment: Text.AlignRight
                text: (rateRangeProxy.value * 100).toFixed(0)
                width: 18
            }
            Skin.ControlFader {
                id: rateSlider

                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: parent.top
                anchors.topMargin: 10
                bar.color: LateNightTheme.deckRateSliderBarColor
                bar.enabled: true
                bar.margin: 7
                bar.start: 0.5
                group: root.group
                height: root.sliderHeight
                key: "rate"
                showDefaultHandle: false
                width: 40

                background: Image {
                    anchors.fill: parent
                    fillMode: Image.PreserveAspectFit
                    source: root.showSyncButton ? LateNightTheme.assetDeckRateCompactSyncSliderBackground : LateNightTheme.assetDeckRateCompactSliderBackground
                }
                handle: Image {
                    fillMode: Image.PreserveAspectFit
                    height: 17
                    source: LateNightTheme.assetDeckRateSliderHandle
                    width: 40
                    x: (rateSlider.width - width) / 2
                    y: rateSlider.visualPosition * (rateSlider.height - height)
                }
            }
            Rectangle {
                border.color: LateNightTheme.deckPanelBorderDark
                color: rateSetDefaultProxy.value > 0 ? LateNightTheme.accentColor : LateNightTheme.textColorMuted
                height: 5
                radius: 1
                width: 5
                x: 4
                y: 10 + (root.sliderHeight - height) / 2
            }
            Text {
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                color: root.displayColor
                font.pixelSize: 11
                horizontalAlignment: Text.AlignHCenter
                text: "+"
                width: 10
            }
            Text {
                anchors.bottom: parent.bottom
                anchors.right: parent.right
                color: root.displayColor
                font.pixelSize: 10
                horizontalAlignment: Text.AlignRight
                text: (rateRangeProxy.value * 100).toFixed(0)
                width: 18
            }
        }
    }
}
