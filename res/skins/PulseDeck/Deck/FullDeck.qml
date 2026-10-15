pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import Mixxx 1.0 as Mixxx
import "../Controls" as Controls
import "../LateNightTheme"

Controls.Panel {
    id: root

    property bool editMode: false
    required property string group
    // Deck B/D place their pad column towards the deck's outer edge, like VirtualDJ.
    property bool mirrorControls: false
    readonly property bool show8Hotcues: show8HotcuesProxy.value > 0
    readonly property bool showBeatjumpControls: showBeatjumpControlsProxy.value > 0
    readonly property bool showBigSpinnyOrCover: SpinnyCoverState.showBigSpinnyOrCover
    readonly property bool showHotcues: showHotcuesProxy.value > 0
    readonly property bool showIntroOutroCues: showIntroOutroCuesProxy.value > 0
    readonly property bool showKeyControls: showKeyControlsProxy.value > 0
    readonly property bool showLoopControls: showLoopControlsProxy.value > 0
    readonly property bool showRateControlButtons: showRateControlButtonsProxy.value > 0
    readonly property bool showRateControls: showRateControlsProxy.value > 0
    readonly property bool showSmallSpinnyOrCover: SpinnyCoverState.showSmallSpinnyOrCover
    readonly property bool showVinylControls: showVinylControlsProxy.value > 0

    signal toggleFocus

    color: LateNightTheme.deckPanelColor
    implicitHeight: LateNightTheme.fullDeckHeight
    implicitWidth: 530


    Mixxx.ControlProxy {
        id: showKeyControlsProxy
        group: "[Skin]"
        key: "show_key_controls"
    }

    Mixxx.ControlProxy {
        id: showVinylControlsProxy
        group: "[Skin]"
        key: "show_vinylcontrol"
    }

    Mixxx.ControlProxy {
        id: show4EffectUnitsProxy
        group: "[Skin]"
        key: "show_4effectunits"
    }

    Mixxx.ControlProxy {
        id: showHotcuesProxy
        group: "[Skin]"
        key: "show_hotcues"
    }

    Mixxx.ControlProxy {
        id: show8HotcuesProxy
        group: "[Skin]"
        key: "show_8_hotcues"
    }

    Mixxx.ControlProxy {
        id: showIntroOutroCuesProxy
        group: "[Skin]"
        key: "show_intro_outro_cues"
    }

    Mixxx.ControlProxy {
        id: showLoopControlsProxy
        group: "[Skin]"
        key: "show_loop_controls"
    }

    Mixxx.ControlProxy {
        id: showBeatjumpControlsProxy
        group: "[Skin]"
        key: "show_beatjump_controls"
    }

    Mixxx.ControlProxy {
        id: showRateControlsProxy
        group: "[Skin]"
        key: "show_rate_controls"
    }

    Mixxx.ControlProxy {
        id: showRateControlButtonsProxy
        group: "[Skin]"
        key: "show_rate_control_buttons"
    }

    RowLayout {
        anchors.fill: parent
        anchors.bottomMargin: LateNightTheme.deckOuterMargin
        anchors.leftMargin: LateNightTheme.deckOuterMargin
        anchors.rightMargin: LateNightTheme.deckOuterMargin
        anchors.topMargin: LateNightTheme.deckOuterMargin
        spacing: 2

        LayoutMirroring.enabled: root.mirrorControls

        // Left control column: FX assignment, hotcue pads, beatjump and intro/outro
        ColumnLayout {
            id: controlColumn
            Layout.fillHeight: true
            Layout.maximumWidth: 176
            Layout.minimumWidth: 176
            Layout.preferredWidth: 176
            spacing: 2

            // Top row: FX assignment and key controls.
            RowLayout {
                id: topPlaceholderRow
                Layout.fillWidth: true
                Layout.preferredHeight: 20
                Layout.minimumHeight: 20
                Layout.maximumHeight: 20
                Layout.fillHeight: false
                spacing: 0

                LateNightFxAssignmentButtons {
                    group: root.group
                    showFourEffectUnits: show4EffectUnitsProxy.value > 0
                }

                Item {
                    Layout.minimumWidth: 2
                    Layout.preferredWidth: 2
                    Layout.maximumWidth: 2
                    Layout.fillHeight: true
                }

                Item {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    Rectangle {
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.bottom: parent.bottom
                        height: 1
                        color: LateNightTheme.deckPanelBorderDark
                    }
                }

                VinylControlsPlaceholder {
                    Layout.minimumWidth: 158
                    Layout.preferredWidth: 158
                    Layout.maximumWidth: 158
                    Layout.preferredHeight: 20
                    Layout.maximumHeight: 20
                    group: root.group
                    visible: root.showVinylControls
                }

                Item {
                    Layout.fillHeight: true
                    Layout.fillWidth: true

                    Rectangle {
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.bottom: parent.bottom
                        height: 1
                        color: LateNightTheme.deckPanelBorderDark
                    }
                }

                Item {
                    Layout.minimumWidth: 2
                    Layout.preferredWidth: 2
                    Layout.maximumWidth: 2
                    Layout.fillHeight: true
                }

                Item {
                    Layout.minimumHeight: 20
                    Layout.preferredHeight: 20
                    Layout.maximumHeight: 20
                    Layout.minimumWidth: 76
                    Layout.preferredWidth: 76
                    Layout.maximumWidth: 111
                    visible: root.showKeyControls

                    KeyControlsPlaceholder {
                        anchors.centerIn: parent
                        group: root.group
                    }
                }
            }

            // Effect section: the three slots of the unit this deck is routed to
            FxColumn {
                id: fxColumn
                Layout.fillWidth: true
                Layout.maximumHeight: 104
                Layout.minimumHeight: 104
                Layout.preferredHeight: 104
                group: root.group
            }

            TransportLoopBeatjumpPlaceholders {
                id: loopColumn
                Layout.fillWidth: true
                Layout.maximumHeight: 52
                Layout.minimumHeight: 52
                Layout.preferredHeight: 52
                flowHeight: 52
                group: root.group
                show8Hotcues: root.show8Hotcues
                showBeatjumpControls: root.showBeatjumpControls
                showHotcues: false
                showIntroOutroCues: root.showIntroOutroCues
                showLoopControls: false
                showTransport: false
            }
        }

        // Central column: title rows, overview, jog and transport
        ColumnLayout {
            id: mainDeckColumn
            Layout.fillHeight: true
            Layout.fillWidth: true
            spacing: 1

            TitleTimeRows {
                id: titleTimeRows
                Layout.fillWidth: true
                Layout.maximumHeight: 46
                Layout.minimumHeight: 46
                Layout.preferredHeight: 46
                group: root.group

                TapHandler {
                    onDoubleTapped: root.toggleFocus()
                }
            }

            // Thin wave overview strip, like the progress bar under the title
            OverviewRow {
                id: overviewStrip
                Layout.fillWidth: true
                Layout.maximumHeight: 18
                Layout.minimumHeight: 18
                Layout.preferredHeight: 18
                // 18 (strip) + 26 (pad row): start the deck settings block right
                // below the hotcue pads so they cannot cover its buttons.
                deckSettingsTopOffset: hotcuePads.visible ? 44 : 18
                group: root.group
            }

            // Hotcue pad bank above the jog, like VirtualDJ's pads
            HotcuePadRow {
                id: hotcuePads
                Layout.fillWidth: true
                Layout.maximumHeight: 26
                Layout.minimumHeight: 26
                Layout.preferredHeight: 26
                group: root.group
                hotcueCount: root.show8Hotcues ? 8 : 4
                padColumns: 8
                visible: root.showHotcues
            }

            // Middle row: the jog wheel, centred like VirtualDJ's platter
            Item {
                id: middleDeckRow
                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.minimumHeight: 60
                Layout.preferredHeight: 104

                // The deck settings column hangs from the overview strip on the
                // right, so the jog is centred in the space left of it and its
                // ring cannot cover the rating stars or the slip/quantize
                // buttons.
                Item {
                    id: jogArea
                    anchors.bottom: parent.bottom
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.rightMargin: LateNightTheme.deckSettingsColumnWidth
                    anchors.top: parent.top

                    // Big Spinny/Cover Slot (Large mode)
                    SpinnyCoverSlot {
                        id: leftSpinnyBig
                        anchors.centerIn: parent
                        group: root.group
                        height: root.showBigSpinnyOrCover ? Math.min(LateNightTheme.fullBigSpinnySize, parent.height, parent.width) : 0
                        visible: root.showBigSpinnyOrCover
                        width: height
                    }

                    // Small Spinny/Cover Slot (Small mode)
                    SpinnyCoverSlot {
                        id: leftSpinnySmall
                        anchors.centerIn: parent
                        group: root.group
                        height: root.showSmallSpinnyOrCover ? Math.min(LateNightTheme.smallSpinnySize, parent.height, parent.width) : 0
                        visible: root.showSmallSpinnyOrCover
                        width: height
                    }
                }
            }

            // Transport and loop controls below the jog
            TransportLoopBeatjumpPlaceholders {
                id: transportRow
                Layout.fillWidth: true
                Layout.maximumHeight: 52
                Layout.minimumHeight: 52
                Layout.preferredHeight: 52
                flowHeight: 52
                group: root.group
                show8Hotcues: root.show8Hotcues
                showBeatjumpControls: false
                showHotcues: false
                showIntroOutroCues: false
                showLoopControls: root.showLoopControls
                showTransport: true
            }
        }

        // Right Rate controls placeholder
        RatePlaceholder {
            id: rateControls
            Layout.fillHeight: false
            Layout.minimumHeight: 202
            Layout.preferredHeight: 202
            Layout.maximumHeight: 202
            Layout.alignment: Qt.AlignTop
            Layout.preferredWidth: root.showRateControls ? 90 : 0
            group: root.group
            showRateControlButtons: root.showRateControlButtons
            visible: root.showRateControls
        }
    }

}
