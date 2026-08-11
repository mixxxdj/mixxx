import QtQuick
import QtQuick.Layouts
import Mixxx 1.0 as Mixxx
import "../Controls" as Controls
import "../LateNightTheme"

Controls.Panel {
    id: root

    property bool editMode: false
    required property string group
    property bool minimized: false
    readonly property bool show8Hotcues: show8HotcuesProxy.value > 0
    readonly property bool showBeatjumpControls: showBeatjumpControlsProxy.value > 0
    readonly property bool showBigSpinnyOrCover: selectBigSpinnyProxy.value > 0
    readonly property bool showHotcues: showHotcuesProxy.value > 0
    readonly property bool showIntroOutroCues: showIntroOutroCuesProxy.value > 0
    readonly property bool showKeyControls: showKeyControlsProxy.value > 0
    readonly property bool showLoopControls: showLoopControlsProxy.value > 0
    readonly property bool showRateControlButtons: showRateControlButtonsProxy.value > 0
    readonly property bool showRateControls: showRateControlsProxy.value > 0
    readonly property bool showSmallSpinnyOrCover: selectBigSpinnyProxy.value <= 0 && !root.minimized
    readonly property bool showVinylControls: showVinylControlsProxy.value > 0

    signal toggleFocus

    color: LateNightTheme.deckPanelColor
    implicitHeight: root.minimized ? 80 : 206
    implicitWidth: 620

    Mixxx.ControlProxy {
        id: selectBigSpinnyProxy

        group: "[Skin]"
        key: "select_big_spinny_or_cover"
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
        anchors.bottomMargin: 2
        anchors.fill: parent
        anchors.leftMargin: 1
        anchors.rightMargin: 1
        anchors.topMargin: 2
        spacing: 2

        // Central main deck column
        ColumnLayout {
            id: mainDeckColumn

            Layout.alignment: Qt.AlignTop
            Layout.fillHeight: false
            Layout.fillWidth: true
            spacing: 1

            // Top row: FX assignment and key controls.
            RowLayout {
                id: topPlaceholderRow

                Layout.fillHeight: false
                Layout.fillWidth: true
                Layout.maximumHeight: 20
                Layout.minimumHeight: 20
                Layout.preferredHeight: 20
                spacing: 0
                visible: !root.minimized

                // FX assignment buttons: toggle effect unit assignment for this deck
                Row {
                    spacing: 0

                    Repeater {
                        model: show4EffectUnitsProxy.value > 0 ? 4 : 2

                        delegate: Item {
                            id: fxAssignButton

                            readonly property bool active: fxAssignProxy.value > 0
                            readonly property color activeColor: index < 2 ? (LateNightTheme.isClassic ? LateNightTheme.effectsUnitColor12 : LateNightTheme.effectsUnitDimColor12) : (LateNightTheme.isClassic ? LateNightTheme.effectsUnitColor34 : LateNightTheme.effectsUnitDimColor34)
                            readonly property color fillColor: active ? activeColor : inactiveColor
                            readonly property color inactiveColor: LateNightTheme.deckEmbeddedButtonInactiveColor
                            required property int index

                            height: 20
                            width: show4EffectUnitsProxy.value > 0 && index > 0 ? 20 : 26

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
                                fillMode: Image.Stretch
                                source: {
                                    if (index === 0) {
                                        return fxAssignButton.active ? LateNightTheme.lateNightButton("btn_embedded_library_active.svg") : LateNightTheme.lateNightButton("btn_embedded_library.svg");
                                    } else {
                                        return fxAssignButton.active ? LateNightTheme.lateNightButton("btn_embedded_grid_active.svg") : LateNightTheme.lateNightButton("btn_embedded_grid.svg");
                                    }
                                }
                            }
                            Text {
                                anchors.centerIn: parent
                                color: fxAssignButton.active ? (LateNightTheme.isClassic ? "#000000" : LateNightTheme.mixerControlTextColor) : (LateNightTheme.isClassic ? "#d2d2d1" : "#666666")
                                font.bold: true
                                font.family: "Open Sans"
                                font.pixelSize: 10
                                text: show4EffectUnitsProxy.value > 0 && index > 0 ? (index + 1).toString() : "FX" + (show4EffectUnitsProxy.value > 0 && index === 0 ? "1" : (index + 1).toString())
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
                    Layout.fillHeight: true
                    Layout.fillWidth: true

                    Rectangle {
                        anchors.bottom: parent.bottom
                        anchors.left: parent.left
                        anchors.right: parent.right
                        color: LateNightTheme.deckPanelBorderDark
                        height: 1
                    }
                }
                VinylControlsPlaceholder {
                    Layout.maximumHeight: 20
                    Layout.preferredHeight: 20
                    Layout.preferredWidth: 158
                    group: root.group
                    visible: root.showVinylControls
                }
                Item {
                    Layout.fillHeight: true
                    Layout.preferredWidth: root.showVinylControls ? 2 : 0
                    visible: root.showVinylControls
                }
                KeyControlsPlaceholder {
                    Layout.maximumHeight: 20
                    Layout.maximumWidth: 111
                    Layout.preferredHeight: 20
                    Layout.preferredWidth: 111
                    group: root.group
                    visible: root.showKeyControls
                }
            }

            // Middle Row: Big Spinny on the left, Title/Overview on the right
            RowLayout {
                id: middleDeckRow

                Layout.fillHeight: false
                Layout.fillWidth: true
                Layout.maximumHeight: root.minimized ? 68 : 122
                Layout.minimumHeight: root.minimized ? 68 : 122
                Layout.preferredHeight: root.minimized ? 68 : 122
                spacing: 8

                // Big Spinny/Cover Slot (Large mode)
                SpinnyCoverSlot {
                    id: leftSpinnyBig

                    Layout.preferredHeight: 114
                    Layout.preferredWidth: 114
                    group: root.group
                    visible: root.showBigSpinnyOrCover && !root.minimized
                }

                // Column containing Title rows and Overview row
                ColumnLayout {
                    id: titleOverviewColumn

                    Layout.fillHeight: false
                    Layout.fillWidth: true
                    Layout.preferredHeight: root.minimized ? 68 : 122
                    spacing: 2

                    // Title, Time, Artist, Duration Rows
                    TitleTimeRows {
                        id: titleTimeRows

                        Layout.fillWidth: true
                        Layout.maximumHeight: root.minimized ? 48 : 55
                        Layout.minimumHeight: root.minimized ? 48 : 55
                        Layout.preferredHeight: root.minimized ? 48 : 55
                        group: root.group

                        TapHandler {
                            onDoubleTapped: root.toggleFocus()
                        }
                    }

                    // Row containing Small Spinny (on the left of overview) and the Waveform Overview
                    RowLayout {
                        id: overviewAndSpinnyRow

                        Layout.fillHeight: false
                        Layout.fillWidth: true
                        Layout.maximumHeight: root.minimized ? 20 : 63
                        Layout.minimumHeight: root.minimized ? 20 : 63
                        Layout.preferredHeight: root.minimized ? 20 : 63
                        spacing: 1

                        // Small Spinny/Cover Slot (Small mode)
                        SpinnyCoverSlot {
                            id: leftSpinnySmall

                            Layout.preferredHeight: 63
                            Layout.preferredWidth: 63
                            group: root.group
                            visible: root.showSmallSpinnyOrCover
                        }

                        // Waveform Overview Row
                        OverviewRow {
                            id: overviewRow

                            Layout.fillHeight: true
                            Layout.fillWidth: true
                            group: root.group
                        }
                    }
                }
            }

            // Lower Transport, Loop, Beatjump Placeholders
            TransportLoopBeatjumpPlaceholders {
                id: transportRow

                Layout.fillHeight: false
                Layout.fillWidth: true
                Layout.maximumHeight: 55
                Layout.minimumHeight: 55
                Layout.preferredHeight: 55
                group: root.group
                show8Hotcues: root.show8Hotcues
                showBeatjumpControls: root.showBeatjumpControls
                showHotcues: root.showHotcues
                showIntroOutroCues: root.showIntroOutroCues
                showLoopControls: root.showLoopControls
                visible: !root.minimized
            }
        }

        // Right Rate controls placeholder
        RatePlaceholder {
            id: rateControls

            Layout.alignment: Qt.AlignTop
            Layout.fillHeight: false
            Layout.maximumHeight: 202
            Layout.minimumHeight: 202
            Layout.preferredHeight: 202
            Layout.preferredWidth: 90
            group: root.group
            showRateControlButtons: root.showRateControlButtons
            visible: !root.minimized && root.showRateControls
        }
    }
    Mixxx.PlayerDropArea {
        anchors.fill: parent
        group: root.group
    }
}
