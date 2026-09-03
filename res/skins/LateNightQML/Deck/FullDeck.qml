import QtQuick
import QtQuick.Layouts
import Mixxx 1.0 as Mixxx
import "../Controls" as Controls
import "../LateNightTheme"

Controls.Panel {
    id: root

    property bool editMode: false
    required property string group
    readonly property bool show8Hotcues: show8HotcuesProxy.value > 0
    readonly property bool showBeatjumpControls: showBeatjumpControlsProxy.value > 0
    readonly property bool showBigSpinnyOrCover: showBigSpinnyOrCoverProxy.initialized
            ? showBigSpinnyOrCoverProxy.value > 0
            : (showSpinnyOrCoverProxy.value > 0 && selectBigSpinnyProxy.value > 0)
    readonly property bool showHotcues: showHotcuesProxy.value > 0
    readonly property bool showIntroOutroCues: showIntroOutroCuesProxy.value > 0
    readonly property bool showKeyControls: showKeyControlsProxy.value > 0
    readonly property bool showLoopControls: showLoopControlsProxy.value > 0
    readonly property bool showRateControlButtons: showRateControlButtonsProxy.value > 0
    readonly property bool showRateControls: showRateControlsProxy.value > 0
    readonly property bool showSmallSpinnyOrCover: showSmallSpinnyOrCoverProxy.initialized
            ? showSmallSpinnyOrCoverProxy.value > 0
            : (showSpinnyOrCoverProxy.value > 0 && selectBigSpinnyProxy.value <= 0)
    readonly property bool showVinylControls: showVinylControlsProxy.value > 0

    signal toggleFocus

    color: LateNightTheme.deckPanelColor
    implicitHeight: LateNightTheme.fullDeckHeight
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
        anchors.leftMargin: LateNightTheme.deckOuterMargin
        anchors.topMargin: LateNightTheme.deckOuterMargin
        anchors.rightMargin: LateNightTheme.deckOuterMargin
        anchors.bottomMargin: LateNightTheme.deckOuterMargin + 1
        spacing: 2

        // Central main deck column
        ColumnLayout {
            id: mainDeckColumn
            Layout.fillWidth: true
            Layout.fillHeight: false
            Layout.alignment: Qt.AlignTop
            spacing: 1

            // Top row: FX assignment and key controls.
            RowLayout {
                id: topPlaceholderRow
                Layout.fillWidth: true
                Layout.preferredHeight: 20
                Layout.minimumHeight: 20
                Layout.maximumHeight: 20
                Layout.fillHeight: false
                spacing: 0

                // FX assignment buttons: toggle effect unit assignment for this deck
                Row {
                    spacing: 0

                    Repeater {
                        model: show4EffectUnitsProxy.value > 0 ? 4 : 2

                        delegate: Item {
                            id: fxAssignButton

                            required property int index

                            width: show4EffectUnitsProxy.value > 0 && index > 0 ? 20 : 26
                            height: 20
                            readonly property bool active: fxAssignProxy.value > 0
                            readonly property color activeColor: index < 2 ? (LateNightTheme.isClassic ? LateNightTheme.effectsUnitColor12 : LateNightTheme.effectsUnitDimColor12) : (LateNightTheme.isClassic ? LateNightTheme.effectsUnitColor34 : LateNightTheme.effectsUnitDimColor34)
                            readonly property color inactiveColor: LateNightTheme.deckEmbeddedButtonInactiveColor
                            readonly property color fillColor: active ? activeColor : inactiveColor

                            Mixxx.ControlProxy {
                                id: fxAssignProxy
                                group: `[EffectRack1_EffectUnit${index + 1}]`
                                key: `group_${root.group}_enable`
                            }

                            Rectangle {
                                anchors.fill: parent
                                color: fxAssignButton.fillColor
                            }

                            Image {
                                anchors.fill: parent
                                source: {
                                    if (index === 0) {
                                        return fxAssignButton.active
                                            ? LateNightTheme.lateNightButton("btn_embedded_library_active.svg")
                                            : LateNightTheme.lateNightButton("btn_embedded_library.svg");
                                    } else {
                                        return fxAssignButton.active
                                            ? LateNightTheme.lateNightButton("btn_embedded_grid_active.svg")
                                            : LateNightTheme.lateNightButton("btn_embedded_grid.svg");
                                    }
                                }
                                fillMode: Image.Stretch
                            }

                            Text {
                                anchors.centerIn: parent
                                text: show4EffectUnitsProxy.value > 0 && index > 0 ? (index + 1).toString() : "FX" + (show4EffectUnitsProxy.value > 0 && index === 0 ? "1" : (index + 1).toString())
                                font.family: "Open Sans"
                                font.pixelSize: 10
                                font.bold: true
                                color: fxAssignButton.active ? (LateNightTheme.isClassic ? "#000000" : LateNightTheme.mixerControlTextColor) : (LateNightTheme.isClassic ? "#d2d2d1" : "#666666")
                            }

                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    fxAssignProxy.value = !fxAssignProxy.value;
                                }
                            }
                        }
                    }
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
                    Layout.preferredWidth: 158
                    Layout.preferredHeight: 20
                    Layout.maximumHeight: 20
                    group: root.group
                    visible: root.showVinylControls
                }

                Item {
                    Layout.preferredWidth: root.showVinylControls ? 2 : 0
                    Layout.fillHeight: true
                    visible: root.showVinylControls
                }

                KeyControlsPlaceholder {
                    Layout.preferredWidth: 111
                    Layout.maximumWidth: 111
                    Layout.preferredHeight: 20
                    Layout.maximumHeight: 20
                    group: root.group
                    visible: root.showKeyControls
                }
            }

            // Middle Row: Big Spinny on the left, Title/Overview on the right
            RowLayout {
                id: middleDeckRow
                Layout.fillWidth: true
                Layout.fillHeight: false
                Layout.minimumHeight: 122
                Layout.preferredHeight: 122
                Layout.maximumHeight: 122
                spacing: 8

                // Big Spinny/Cover Slot (Large mode)
                SpinnyCoverSlot {
                    id: leftSpinnyBig
                    Layout.preferredHeight: root.showBigSpinnyOrCover ? LateNightTheme.fullBigSpinnySize : 0
                    Layout.preferredWidth: root.showBigSpinnyOrCover ? LateNightTheme.fullBigSpinnySize : 0
                    group: root.group
                    visible: root.showBigSpinnyOrCover
                }

                // Column containing Title rows and Overview row
                ColumnLayout {
                    id: titleOverviewColumn
                    Layout.fillWidth: true
                    Layout.fillHeight: false
                    Layout.preferredHeight: 122
                    spacing: 2

                    // Title, Time, Artist, Duration Rows
                    TitleTimeRows {
                        id: titleTimeRows
                        Layout.fillWidth: true
                        Layout.minimumHeight: 55
                        Layout.preferredHeight: 55
                        Layout.maximumHeight: 55
                        group: root.group

                        TapHandler {
                            onDoubleTapped: root.toggleFocus()
                        }
                    }

                    // Row containing Small Spinny (on the left of overview) and the Waveform Overview
                    RowLayout {
                        id: overviewAndSpinnyRow
                        Layout.fillWidth: true
                        Layout.fillHeight: false
                        Layout.minimumHeight: 63
                        Layout.preferredHeight: 63
                        Layout.maximumHeight: 63
                        spacing: 1

                        // Small Spinny/Cover Slot (Small mode)
                        SpinnyCoverSlot {
                            id: leftSpinnySmall
                            Layout.preferredHeight: root.showSmallSpinnyOrCover ? LateNightTheme.smallSpinnySize : 0
                            Layout.preferredWidth: root.showSmallSpinnyOrCover ? LateNightTheme.smallSpinnySize : 0
                            group: root.group
                            visible: root.showSmallSpinnyOrCover
                        }

                        // Waveform Overview Row
                        OverviewRow {
                            id: overviewRow
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            group: root.group
                        }
                    }
                }
            }

            // Lower Transport, Loop, Beatjump Placeholders
            TransportLoopBeatjumpPlaceholders {
                id: transportRow
                Layout.fillWidth: true
                Layout.fillHeight: false
                Layout.minimumHeight: LateNightTheme.deckTransportHeight
                Layout.preferredHeight: LateNightTheme.deckTransportHeight
                Layout.maximumHeight: LateNightTheme.deckTransportHeight
                group: root.group
                showHotcues: root.showHotcues
                show8Hotcues: root.show8Hotcues
                showIntroOutroCues: root.showIntroOutroCues
                showLoopControls: root.showLoopControls
                showBeatjumpControls: root.showBeatjumpControls
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
