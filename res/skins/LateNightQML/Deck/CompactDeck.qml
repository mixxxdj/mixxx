import QtQuick
import QtQuick.Layouts
import Mixxx 1.0 as Mixxx
import "../Controls" as Controls
import "../LateNightTheme"

Controls.Panel {
    id: root

    required property string group
    readonly property bool showBeatjumpControls: showBeatjumpControlsProxy.value > 0
    readonly property bool showBigSpinnyOrCover: showBigSpinnyOrCoverProxy.initialized
            ? showBigSpinnyOrCoverProxy.value > 0
            : (showSpinnyOrCoverProxy.value > 0 && selectBigSpinnyProxy.value > 0)
    readonly property bool showKeyControls: showKeyControlsProxy.value > 0
    readonly property bool showLoopControls: showLoopControlsProxy.value > 0
    readonly property bool showRateControls: showRateControlsProxy.value > 0
    readonly property bool showSmallSpinnyOrCover: showSmallSpinnyOrCoverProxy.initialized
            ? showSmallSpinnyOrCoverProxy.value > 0
            : (showSpinnyOrCoverProxy.value > 0 && selectBigSpinnyProxy.value <= 0)
    readonly property bool showVinylControls: showVinylControlsProxy.value > 0

    signal toggleFocus

    color: LateNightTheme.deckPanelColor
    implicitHeight: LateNightTheme.compactDeckHeight
    implicitWidth: 620

    Mixxx.ControlProxy {
        id: selectBigSpinnyProxy

        group: "[Skin]"
        key: "select_big_spinny_or_cover"
    }
    Mixxx.ControlProxy {
        id: showSpinnyOrCoverProxy

        group: "[Skin]"
        key: "show_spinny_or_cover"
    }
    Mixxx.ControlProxy {
        id: showSmallSpinnyOrCoverProxy

        group: "[Skin]"
        key: "show_small_spinny_or_cover"
    }
    Mixxx.ControlProxy {
        id: showBigSpinnyOrCoverProxy

        group: "[Skin]"
        key: "show_big_spinny_or_cover"
    }
    Mixxx.ControlProxy {
        id: showVinylControlsProxy

        group: "[Skin]"
        key: "show_vinylcontrol"
    }
    Mixxx.ControlProxy {
        id: showKeyControlsProxy

        group: "[Skin]"
        key: "show_key_controls_compact"
    }
    Mixxx.ControlProxy {
        id: showLoopControlsProxy

        group: "[Skin]"
        key: "show_loop_controls_compact"
    }
    Mixxx.ControlProxy {
        id: showBeatjumpControlsProxy

        group: "[Skin]"
        key: "show_beatjump_controls_compact"
    }
    Mixxx.ControlProxy {
        id: showRateControlsProxy

        group: "[Skin]"
        key: "show_rate_controls_compact"
    }
    RowLayout {
        anchors.bottomMargin: LateNightTheme.deckOuterMargin
        anchors.fill: parent
        anchors.leftMargin: LateNightTheme.deckOuterMargin
        anchors.rightMargin: LateNightTheme.deckOuterMargin
        anchors.topMargin: LateNightTheme.deckOuterMargin
        spacing: 2

        ColumnLayout {
            Layout.fillHeight: true
            Layout.fillWidth: true
            spacing: 1

            RowLayout {
                Layout.fillWidth: true
                Layout.preferredHeight: 110
                spacing: 2

                SpinnyCoverSlot {
                    Layout.preferredHeight: root.showBigSpinnyOrCover ? LateNightTheme.compactBigSpinnySize : 0
                    Layout.preferredWidth: root.showBigSpinnyOrCover ? LateNightTheme.compactBigSpinnySize : 0
                    group: root.group
                    visible: root.showBigSpinnyOrCover
                }
                ColumnLayout {
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    spacing: 1

                    TitleTimeRowsCompact {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 46
                        group: root.group

                        TapHandler {
                            onDoubleTapped: root.toggleFocus()
                        }
                    }
                    RowLayout {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 63
                        spacing: 1

                        SpinnyCoverSlot {
                            Layout.preferredHeight: root.showSmallSpinnyOrCover ? LateNightTheme.smallSpinnySize : 0
                            Layout.preferredWidth: root.showSmallSpinnyOrCover ? LateNightTheme.smallSpinnySize : 0
                            group: root.group
                            visible: root.showSmallSpinnyOrCover
                        }
                        OverviewRow {
                            Layout.fillHeight: true
                            Layout.fillWidth: true
                            group: root.group
                        }
                    }
                }
            }
            TransportLoopBeatjumpCompact {
                Layout.fillWidth: true
                Layout.preferredHeight: LateNightTheme.deckTransportHeight
                group: root.group
                showBeatjumpControls: root.showBeatjumpControls
                showKeyControls: root.showKeyControls
                showLoopControls: root.showLoopControls
                showVinylControls: root.showVinylControls
            }
        }
        RateCompact {
            Layout.fillHeight: true
            Layout.preferredWidth: root.showRateControls ? 62 : 0
            group: root.group
            visible: root.showRateControls
        }
    }
}
