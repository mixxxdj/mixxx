pragma ComponentBehavior: Bound

import "../Controls" as Controls
import "../Deck" as DeckControls
import "../LateNightTheme"
import "../../../qml" as Shared
import "../../../qml/Mixxx/Controls" as MixxxControls
import Mixxx 1.0 as Mixxx
import QtQuick
import QtQuick.Layouts

Controls.Panel {
    id: root

    property var currentTrack: player?.currentTrack ?? null
    required property string group
    readonly property bool loaded: trackLoadedControl.value > 0
    property var player: Mixxx.PlayerManager.getPlayer(group)
    property bool show8Hotcues: true
    property bool showFxAssignments: true
    readonly property int tempoGainVuWidth: 52
    readonly property bool useFilteredOverview: Math.round(waveformOverviewTypeControl.value) === 0
    property bool waveformsActive: true

    bottomBorderColor: LateNightTheme.mixerPanelBorderBottom
    clip: true
    color: LateNightTheme.samplerPanelColor
    implicitHeight: 98
    implicitWidth: 280
    leftBorderColor: LateNightTheme.mixerPanelBorderLeft
    radius: LateNightTheme.isClassic ? 2 : 1
    rightBorderColor: LateNightTheme.mixerPanelBorderRight
    topBorderColor: LateNightTheme.mixerPanelBorderTop

    RowLayout {
        anchors.bottomMargin: LateNightTheme.isClassic ? 3 : 1
        anchors.fill: parent
        anchors.leftMargin: LateNightTheme.isClassic ? 3 : 2
        anchors.rightMargin: 1
        anchors.topMargin: LateNightTheme.isClassic ? 2 : 1
        spacing: 0

        ColumnLayout {
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.minimumHeight: 0
            Layout.minimumWidth: 0
            spacing: 0

            RowLayout {
                Layout.fillWidth: true
                Layout.maximumHeight: 28
                Layout.minimumHeight: 28
                Layout.minimumWidth: 0
                Layout.preferredHeight: 28
                spacing: 0

                Text {
                    Layout.fillWidth: true
                    Layout.leftMargin: LateNightTheme.isClassic ? 0 : 4
                    Layout.minimumWidth: 0
                    Layout.rightMargin: 3
                    color: LateNightTheme.samplerTitleColor
                    elide: Text.ElideRight
                    font.family: "Open Sans"
                    font.pixelSize: 16
                    font.weight: Font.Medium
                    text: root.currentTrack?.title ?? ""
                    verticalAlignment: Text.AlignVCenter
                }
                Item {
                    Layout.fillHeight: true
                    Layout.preferredWidth: root.tempoGainVuWidth

                    Rectangle {
                        anchors.bottom: parent.bottom
                        anchors.left: parent.left
                        anchors.top: parent.top
                        color: LateNightTheme.deckPanelBorderDark
                        width: 1
                    }
                    Rectangle {
                        anchors.bottom: parent.bottom
                        anchors.left: parent.left
                        anchors.leftMargin: 1
                        anchors.top: parent.top
                        color: LateNightTheme.samplerBpmSeparatorLightColor
                        width: 1
                    }
                    Text {
                        anchors.bottom: parent.bottom
                        anchors.left: parent.left
                        anchors.leftMargin: 2
                        anchors.right: parent.right
                        anchors.rightMargin: 1
                        anchors.top: parent.top
                        color: LateNightTheme.samplerBpmColor
                        font.family: "Open Sans"
                        font.pixelSize: 14
                        font.weight: Font.Medium
                        horizontalAlignment: Text.AlignHCenter
                        text: root.loaded && visualBpmControl.value > 0 ? visualBpmControl.value.toFixed(2) : ""
                        verticalAlignment: Text.AlignVCenter
                    }
                    Rectangle {
                        anchors.bottom: parent.bottom
                        anchors.right: parent.right
                        anchors.top: parent.top
                        color: LateNightTheme.isClassic ? LateNightTheme.deckPanelBorderDark : LateNightTheme.samplerBpmSeparatorLightColor
                        width: 1
                    }
                }
            }
            RowLayout {
                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.minimumHeight: 0
                Layout.minimumWidth: 0
                spacing: 0

                ColumnLayout {
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    Layout.minimumHeight: 0
                    Layout.minimumWidth: 0
                    spacing: 0

                    RowLayout {
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        Layout.minimumHeight: 0
                        Layout.minimumWidth: 0
                        spacing: 0

                        Item {
                            Layout.fillHeight: true
                            Layout.preferredWidth: 36

                            DeckControls.LateNightControlButton {
                                activeColor: LateNightTheme.activePlayCueColor
                                anchors.left: parent.left
                                anchors.verticalCenter: parent.verticalCenter
                                backgroundSource: LateNightTheme.lateNightTopRegionButton("square_big")
                                contentOpacity: 1
                                displayKey: "play_latched"
                                group: root.group
                                height: 34
                                iconSource: isActive ? LateNightTheme.assetSamplerPauseButton : LateNightTheme.assetSamplerPlayButton
                                inactiveColor: LateNightTheme.deckButtonInactiveColor
                                inactiveFillEnabled: true
                                inactiveOpacity: 1
                                key: "cue_gotoandplay"
                                rightClickKey: "cue_default"
                                stretchIcon: true
                                width: 34
                            }
                            Rectangle {
                                anchors.left: parent.left
                                anchors.verticalCenter: parent.verticalCenter
                                border.color: LateNightTheme.activePlayCueColor
                                border.width: 3
                                color: "transparent"
                                height: 34
                                visible: playControl.value > 0 && playLatchedControl.value <= 0
                                width: 34
                            }
                        }
                        Rectangle {
                            Layout.fillHeight: true
                            Layout.fillWidth: true
                            Layout.minimumWidth: 0
                            Layout.preferredWidth: 145
                            color: root.loaded ? LateNightTheme.samplerOverviewBackgroundLoadedColor : LateNightTheme.samplerOverviewBackgroundColor

                            Loader {
                                active: root.loaded && root.waveformsActive
                                anchors.fill: parent
                                anchors.margins: 1
                                asynchronous: true

                                sourceComponent: Component {
                                    MixxxControls.WaveformOverview {
                                        channels: Mixxx.WaveformOverview.Channels.BothChannels
                                        colorHigh: root.useFilteredOverview ? LateNightTheme.samplerWaveformFilteredHighColor : LateNightTheme.overviewRgbLowColor
                                        colorLow: root.useFilteredOverview ? LateNightTheme.samplerWaveformFilteredLowColor : LateNightTheme.overviewRgbHighColor
                                        colorMid: root.useFilteredOverview ? LateNightTheme.samplerWaveformFilteredMidColor : LateNightTheme.overviewRgbMidColor
                                        cueMarkerColor: LateNightTheme.waveformCueColor
                                        group: root.group
                                        introOutroMarkerColor: LateNightTheme.waveformIntroOutroColor
                                        loopMarkerColor: LateNightTheme.waveformLoopColor
                                        playPositionMarkerColor: LateNightTheme.waveformPlayPositionColor.toString()
                                        renderer: root.useFilteredOverview ? Mixxx.WaveformOverview.Renderer.Filtered : Mixxx.WaveformOverview.Renderer.RGB
                                    }
                                }
                            }
                            Rectangle {
                                anchors.left: parent.left
                                anchors.right: parent.right
                                anchors.top: parent.top
                                color: LateNightTheme.samplerOverviewBorderTopColor
                                height: 1
                            }
                            Rectangle {
                                anchors.bottom: parent.bottom
                                anchors.left: parent.left
                                anchors.top: parent.top
                                color: LateNightTheme.samplerOverviewBorderLeftColor
                                width: 1
                            }
                            Rectangle {
                                anchors.bottom: parent.bottom
                                anchors.left: parent.left
                                anchors.right: parent.right
                                color: LateNightTheme.samplerOverviewBorderBottomColor
                                height: 1
                            }
                            Rectangle {
                                anchors.bottom: parent.bottom
                                anchors.right: parent.right
                                anchors.top: parent.top
                                color: LateNightTheme.samplerOverviewBorderRightColor
                                width: 1
                            }
                        }
                        Rectangle {
                            Layout.fillHeight: true
                            Layout.preferredWidth: 43
                            color: LateNightTheme.overviewSettingsBackgroundColor

                            Rectangle {
                                anchors.left: parent.left
                                anchors.right: parent.right
                                anchors.top: parent.top
                                color: LateNightTheme.samplerSettingsBorderTopColor
                                height: 1
                            }
                            Rectangle {
                                anchors.bottom: parent.bottom
                                anchors.left: parent.left
                                anchors.right: parent.right
                                color: LateNightTheme.samplerSettingsBorderBottomColor
                                height: 1
                            }
                            GridLayout {
                                anchors.horizontalCenter: parent.horizontalCenter
                                anchors.verticalCenter: parent.verticalCenter
                                columnSpacing: 0
                                columns: 2
                                rowSpacing: 0

                                DeckControls.LateNightControlButton {
                                    Layout.preferredHeight: 18
                                    Layout.preferredWidth: 21
                                    activeIconSuffix: "active_12"
                                    backgroundSource: ""
                                    contentOpacity: 1
                                    group: root.group
                                    iconSource: LateNightTheme.lateNightAsset("buttons", "btn__repeat.svg")
                                    inactiveFillEnabled: false
                                    key: "repeat"
                                    stretchIcon: true
                                    toggleable: true
                                }
                                DeckControls.LateNightControlButton {
                                    Layout.preferredHeight: 18
                                    Layout.preferredWidth: 21
                                    activeIconSuffix: "active_12"
                                    backgroundSource: ""
                                    contentOpacity: 1
                                    group: root.group
                                    iconSource: LateNightTheme.lateNightAsset("buttons", "btn__eject.svg")
                                    inactiveFillEnabled: false
                                    key: "eject"
                                    stretchIcon: true
                                }
                                DeckControls.LateNightControlButton {
                                    Layout.preferredHeight: 18
                                    Layout.preferredWidth: 21
                                    activeIconSuffix: "active_12"
                                    backgroundSource: ""
                                    contentOpacity: 1
                                    group: root.group
                                    iconSource: LateNightTheme.lateNightAsset("buttons", "btn__keylock.svg")
                                    inactiveFillEnabled: false
                                    key: "keylock"
                                    stretchIcon: true
                                    toggleable: true
                                }
                                SamplerOrientationButton {
                                    Layout.preferredHeight: 18
                                    Layout.preferredWidth: 21
                                    group: root.group
                                }
                            }
                        }
                    }
                    Item {
                        Layout.fillWidth: true
                        Layout.minimumWidth: 0
                        Layout.preferredHeight: 29
                        clip: true

                        Image {
                            anchors.fill: parent
                            fillMode: Image.Tile
                            source: LateNightTheme.optionalDeckControlsBackgroundTile
                            visible: source.toString().length > 0
                        }
                        RowLayout {
                            id: samplerButtonsRow

                            readonly property real buttonScale: idealButtonsWidth > 0 ? Math.min(1, width / idealButtonsWidth) : 1
                            readonly property int effectButtonCount: root.showFxAssignments ? (show4EffectUnitsControl.value > 0 ? 4 : 2) : 0
                            readonly property real effectButtonsWidth: effectButtonCount === 4 ? 86 : effectButtonCount === 2 ? 52 : 0
                            readonly property int hotcueButtonCount: root.show8Hotcues ? 8 : 4
                            readonly property real idealButtonsWidth: hotcueButtonCount * 26 + effectButtonsWidth

                            anchors.bottomMargin: 2
                            anchors.fill: parent
                            anchors.leftMargin: LateNightTheme.isClassic ? 1 : 0
                            anchors.rightMargin: LateNightTheme.isClassic ? 1 : 0
                            anchors.topMargin: 1
                            spacing: 0

                            Repeater {
                                model: root.show8Hotcues ? 8 : 4

                                SamplerHotcueButton {
                                    required property int index

                                    Layout.alignment: Qt.AlignVCenter
                                    Layout.minimumHeight: 0
                                    Layout.minimumWidth: 0
                                    Layout.preferredHeight: 26 * samplerButtonsRow.buttonScale
                                    Layout.preferredWidth: 26 * samplerButtonsRow.buttonScale
                                    group: root.group
                                    hotcueNumber: index + 1
                                }
                            }
                            Item {
                                Layout.fillWidth: true
                                Layout.minimumWidth: 0
                            }
                            Repeater {
                                model: samplerButtonsRow.effectButtonCount

                                DeckControls.LateNightControlButton {
                                    required property int index

                                    Layout.alignment: Qt.AlignVCenter
                                    Layout.minimumHeight: 0
                                    Layout.minimumWidth: 0
                                    Layout.preferredHeight: 20 * samplerButtonsRow.buttonScale
                                    Layout.preferredWidth: (show4EffectUnitsControl.value > 0 && index > 0 ? 20 : 26) * samplerButtonsRow.buttonScale
                                    activeBackgroundSuffix: "active"
                                    activeColor: index < 2 ? LateNightTheme.samplerEffectAssignment12ActiveColor : LateNightTheme.samplerEffectAssignment34ActiveColor
                                    backgroundBorderBottom: 2
                                    backgroundBorderLeft: index === 0 ? 2 : 1
                                    backgroundBorderRight: 2
                                    backgroundBorderTop: 2
                                    backgroundSource: LateNightTheme.lateNightAsset("buttons", index === 0 ? "btn_embedded_library.svg" : "btn_embedded_grid.svg")
                                    fillMargin: 0
                                    fillRadius: 0
                                    group: "[EffectRack1_EffectUnit" + (index + 1) + "]"
                                    inactiveColor: LateNightTheme.deckEmbeddedButtonInactiveColor
                                    inactiveOpacity: 1
                                    key: "group_" + root.group + "_enable"
                                    label: show4EffectUnitsControl.value > 0 ? (index === 0 ? "\u200aFX\u200a1" : String(index + 1)) : "\u200aFX" + (index + 1)
                                    labelColor: isActive ? LateNightTheme.deckActiveButtonTextColor : LateNightTheme.samplerEffectAssignmentInactiveTextColor
                                    labelPixelSize: 12
                                    solidFillEnabled: true
                                    toggleable: true
                                    useBorderImageBackground: true
                                }
                            }
                            Item {
                                Layout.fillWidth: true
                                Layout.minimumWidth: 0
                            }
                        }
                    }
                }
                Rectangle {
                    Layout.fillHeight: true
                    Layout.minimumHeight: 0
                    Layout.preferredWidth: root.tempoGainVuWidth
                    clip: true
                    color: "transparent"

                    Rectangle {
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.top: parent.top
                        color: LateNightTheme.deckPanelBorderDark
                        height: 1
                    }
                    Rectangle {
                        anchors.bottom: parent.bottom
                        anchors.left: parent.left
                        anchors.top: parent.top
                        color: LateNightTheme.deckPanelBorderDark
                        width: 1
                    }
                    Rectangle {
                        anchors.bottom: parent.bottom
                        anchors.right: parent.right
                        anchors.top: parent.top
                        color: LateNightTheme.isClassic ? LateNightTheme.deckPanelBorderDark : LateNightTheme.samplerBpmSeparatorLightColor
                        width: 1
                    }
                    RowLayout {
                        anchors.bottomMargin: 2
                        anchors.fill: parent
                        anchors.leftMargin: LateNightTheme.isClassic ? 3 : 1
                        anchors.rightMargin: 3
                        anchors.topMargin: LateNightTheme.isClassic ? 3 : 1
                        spacing: 0

                        ColumnLayout {
                            Layout.fillHeight: true
                            Layout.fillWidth: true
                            Layout.minimumHeight: 0
                            spacing: 0

                            SamplerGainKnob {
                                Layout.alignment: Qt.AlignHCenter
                                Layout.preferredHeight: 30
                                Layout.preferredWidth: 35
                                group: root.group
                            }
                            Item {
                                Layout.fillHeight: true
                            }
                            DeckControls.LateNightControlButton {
                                Layout.alignment: Qt.AlignHCenter
                                Layout.preferredHeight: 26
                                Layout.preferredWidth: 26
                                activeColor: LateNightTheme.samplerPflActiveColor
                                activeIconSuffix: LateNightTheme.isPaleMoon ? "active" : ""
                                backgroundSource: LateNightTheme.lateNightTopRegionButton("square")
                                contentOpacity: 1
                                fillMargin: 1
                                fillRadius: 2
                                group: root.group
                                iconSource: LateNightTheme.lateNightAsset("buttons", "btn__pfl.svg")
                                inactiveColor: LateNightTheme.deckEmbeddedButtonInactiveColor
                                inactiveOpacity: 1
                                key: "pfl"
                                solidFillEnabled: true
                                stretchIcon: true
                                toggleable: true
                            }
                        }
                        SamplerVuMeter {
                            Layout.alignment: Qt.AlignVCenter
                            Layout.preferredHeight: 62
                            Layout.preferredWidth: 8
                            group: root.group
                        }
                    }
                }
            }
        }
        ColumnLayout {
            Layout.alignment: Qt.AlignTop
            Layout.preferredWidth: 32
            spacing: 0

            DeckControls.LateNightControlButton {
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredHeight: 26
                Layout.preferredWidth: 26
                activeColor: LateNightTheme.activePlayCueColor
                activeIconSuffix: LateNightTheme.isPaleMoon ? "active" : ""
                backgroundSource: LateNightTheme.lateNightTopRegionButton("square")
                contentOpacity: 1
                fillMargin: 1
                fillRadius: 2
                group: root.group
                iconSource: LateNightTheme.assetSamplerSyncButton
                inactiveColor: LateNightTheme.deckEmbeddedButtonInactiveColor
                inactiveOpacity: 1
                key: "sync_enabled"
                solidFillEnabled: true
                stretchIcon: true
                toggleable: true
            }
            Shared.ControlFader {
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredHeight: 65
                Layout.preferredWidth: 30
                backgroundMargin: 0
                bar.color: LateNightTheme.samplerPitchSliderBarColor
                bar.margin: 5
                bar.start: 0.5
                bg: LateNightTheme.assetSamplerPitchSliderBackground
                fg: LateNightTheme.assetSamplerPitchSliderHandle
                group: root.group
                key: "rate"
                orientation: Qt.Vertical
                showHandleShadow: false
            }
        }
    }
    Mixxx.ControlProxy {
        id: trackLoadedControl

        group: root.group
        key: "track_loaded"
    }
    Mixxx.ControlProxy {
        id: visualBpmControl

        group: root.group
        key: "visual_bpm"
    }
    Mixxx.ControlProxy {
        id: playControl

        group: root.group
        key: "play"
    }
    Mixxx.ControlProxy {
        id: playLatchedControl

        group: root.group
        key: "play_latched"
    }
    Mixxx.ControlProxy {
        id: show4EffectUnitsControl

        group: "[Skin]"
        key: "show_4effectunits"
    }
    Mixxx.ControlProxy {
        id: waveformOverviewTypeControl

        group: "[Waveform]"
        key: "WaveformOverviewType"
    }
    Mixxx.PlayerDropArea {
        anchors.fill: parent
        group: root.group
    }
}
