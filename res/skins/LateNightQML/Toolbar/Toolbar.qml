pragma ComponentBehavior: Bound

import "../LateNightTheme"
import "../../../qml" as Shared
import Mixxx 1.0 as Mixxx
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root

    property bool editDeck: false
    property alias maximizeLibrary: maximizeLibraryButton.checked
    readonly property bool show4decks: show4DecksButton.checked && show4DecksButton.visible
    property bool show4decksAvailable: true
    property alias showMaximizedDecks: maxLibraryDecksButton.checked
    property alias showEffects: showEffectsButton.checked
    property alias showMicAux: showMicAuxButton.checked
    property alias showMixer: showMixerButton.checked
    property alias showSamplers: showSamplersButton.checked
    property alias showWaveforms: showWaveformsButton.checked

    function formatTime(date) {
        const hours = date.getHours();
        const displayHour = hours % 12 || 12;
        const suffix = hours >= 12 ? "PM" : "AM";
        return displayHour.toString() + ":" + date.getMinutes().toString().padStart(2, "0") + " " + suffix;
    }
    function initializeToolbarDefaults() {
        if (!toolbarDefaultsControl.initialized || toolbarDefaultsControl.value >= 1.0) {
            return;
        }
        setControlValueIfInitialized(showMixerControl, 1.0);
        setControlValueIfInitialized(showWaveformsControl, 1.0);
        showEffectRackControl.value = 1.0;
        showMicAuxControl.value = 0.0;
        showSamplersControl.value = 0.0;
        setControlValueIfInitialized(show4DecksControl, 0.0);
        showMaximizedLibraryControl.value = 0.0;
        maxLibraryDecksControl.value = 1.0;
        deckSizeControl.value = 1.0;
        eqKnobsControl.value = 1.0;
        eqKillsControl.value = 1.0;
        crossfaderControl.value = 1.0;
        mainHeadMixerControl.value = 1.0;
        show4EffectUnitsControl.value = 0.0;
        showSuperKnobsControl.value = 0.0;
        showPreviewDecksControl.value = 0.0;
        showLibraryCoverArtControl.value = 1.0;
        samplerRowsControl.value = 1.0;
        showHotcuesControl.value = 1.0;
        show8HotcuesControl.value = 1.0;
        showIntroOutroCuesControl.value = 1.0;
        showLoopControlsControl.value = 1.0;
        showBeatjumpControlsControl.value = 1.0;
        showRateControlsControl.value = 1.0;
        showRateControlButtonsControl.value = 1.0;
        showKeyControlsControl.value = 1.0;
        showVinylControlsControl.value = 0.0;
        showSpinniesControl.value = 1.0;
        showCoverArtControl.value = 1.0;
        selectBigSpinnyOrCoverControl.value = 0.0;
        toolbarDefaultsControl.value = 1.0;
    }
    function openPopupForButton(popup, button) {
        const mapped = button.mapToItem(root, 0, 0);
        popup.x = Math.max(0, Math.min(root.width - popup.width, mapped.x));
        popup.y = root.height + 2;
        popup.open();
    }
    function setControlValueIfInitialized(control, value) {
        if (control.initialized) {
            control.value = value;
        }
    }
    function setShowWaveforms(enabled) {
        showWaveformsButton.checked = enabled;
        setControlValueIfInitialized(showWaveformsControl, enabled ? 1.0 : 0.0);
    }
    function syncShow4Decks(enabled) {
        show4DecksButton.checked = enabled;
        maximizedShow4DecksButton.checked = enabled;
    }
    function setShow4Decks(enabled) {
        syncShow4Decks(enabled);
        setControlValueIfInitialized(show4DecksControl, enabled ? 1.0 : 0.0);
    }
    function setShowMixer(enabled) {
        showMixerButton.checked = enabled;
        setControlValueIfInitialized(showMixerControl, enabled ? 1.0 : 0.0);
    }
    function setShowMaximizedDecks(enabled) {
        maxLibraryDecksButton.checked = enabled;
        setControlValueIfInitialized(maxLibraryDecksControl, enabled ? 1.0 : 0.0);
    }
    function setDeckSize(size) {
        deckSizeControl.value = size;
        showFullDeckControl.value = size === 2.0 ? 1.0 : 0.0;
        showCompactDeckControl.value = size === 1.0 ? 1.0 : 0.0;
        showMiniDeckControl.value = size === 0.0 ? 1.0 : 0.0;
    }
    function broadcastBackgroundColor(status) {
        if (status === 1.0) {
            return LateNightTheme.toolbarRecordInitColor;
        }
        if (status === 2.0) {
            return LateNightTheme.toolbarBroadcastOnColor;
        }
        if (status === 3.0 || status === 4.0) {
            return LateNightTheme.toolbarStatusErrorColor;
        }
        return LateNightTheme.toolbarButtonInactiveBackgroundColor;
    }
    function recordingBackgroundColor(status) {
        if (status === 1.0) {
            return LateNightTheme.toolbarRecordInitColor;
        }
        if (status === 2.0 || status === 3.0) {
            return LateNightTheme.toolbarRecordOnColor;
        }
        return LateNightTheme.toolbarButtonInactiveBackgroundColor;
    }

    color: "#151517"
    height: 26

    Rectangle {
        anchors.bottom: parent.bottom
        color: "#020202"
        height: 1
        width: parent.width
    }
    Mixxx.ControlProxy {
        id: toolbarDefaultsControl

        group: "[LateNightQML]"
        key: "initialized_toolbar_defaults"

        onInitializedChanged: {
            root.initializeToolbarDefaults();
        }
        onValueChanged: {
            root.initializeToolbarDefaults();
        }
    }
    Mixxx.ControlProxy {
        id: showMixerControl

        group: "[Skin]"
        key: "show_mixer"

        onValueChanged: {
            showMixerButton.checked = value > 0;
        }
    }
    Mixxx.ControlProxy {
        id: showWaveformsControl

        group: "[Skin]"
        key: "show_waveforms"

        onValueChanged: {
            showWaveformsButton.checked = value > 0;
        }
    }
    Mixxx.ControlProxy {
        id: showEffectRackControl

        group: "[Skin]"
        key: "show_effectrack"

        onValueChanged: {
            showEffectsButton.checked = value > 0;
        }
    }
    Mixxx.ControlProxy {
        id: showSamplersControl

        group: "[Skin]"
        key: "show_samplers"

        onValueChanged: {
            showSamplersButton.checked = value > 0;
        }
    }
    Mixxx.ControlProxy {
        id: showMicAuxControl

        group: "[Skin]"
        key: "show_microphones"

        onValueChanged: {
            showMicAuxButton.checked = value > 0;
        }
    }
    Mixxx.ControlProxy {
        id: show4DecksControl

        group: "[Skin]"
        key: "show_4decks"

        onValueChanged: {
            root.syncShow4Decks(value > 0);
        }
    }
    Mixxx.ControlProxy {
        id: showMaximizedLibraryControl

        group: "[Skin]"
        key: "show_maximized_library"

        onValueChanged: {
            maximizeLibraryButton.checked = value > 0;
        }
    }
    Mixxx.ControlProxy {
        id: maxLibraryDecksControl

        group: "[LateNight]"
        key: "max_lib_show_decks"

        onInitializedChanged: {
            maxLibraryDecksButton.checked = value > 0;
        }
        onValueChanged: {
            maxLibraryDecksButton.checked = value > 0;
        }
    }
    Mixxx.ControlProxy {
        id: deckSizeControl

        group: "[LateNight]"
        key: "deck_size_without_mixer"
    }
    Mixxx.ControlProxy {
        id: showFullDeckControl

        group: "[LateNight]"
        key: "show_full_deck"
    }
    Mixxx.ControlProxy {
        id: showCompactDeckControl

        group: "[LateNight]"
        key: "show_compact_deck"
    }
    Mixxx.ControlProxy {
        id: showMiniDeckControl

        group: "[LateNight]"
        key: "show_mini_deck"
    }
    Mixxx.ControlProxy {
        id: showHotcuesControl

        group: "[Skin]"
        key: "show_hotcues"
    }
    Mixxx.ControlProxy {
        id: show8HotcuesControl

        group: "[Skin]"
        key: "show_8_hotcues"
    }
    Mixxx.ControlProxy {
        id: showIntroOutroCuesControl

        group: "[Skin]"
        key: "show_intro_outro_cues"
    }
    Mixxx.ControlProxy {
        id: showLoopControlsControl

        group: "[Skin]"
        key: "show_loop_controls"
    }
    Mixxx.ControlProxy {
        id: showBeatjumpControlsControl

        group: "[Skin]"
        key: "show_beatjump_controls"
    }
    Mixxx.ControlProxy {
        id: showRateControlsControl

        group: "[Skin]"
        key: "show_rate_controls"
    }
    Mixxx.ControlProxy {
        id: showRateControlButtonsControl

        group: "[Skin]"
        key: "show_rate_control_buttons"
    }
    Mixxx.ControlProxy {
        id: showKeyControlsControl

        group: "[Skin]"
        key: "show_key_controls"
    }
    Mixxx.ControlProxy {
        id: showVinylControlsControl

        group: "[Skin]"
        key: "show_vinylcontrol"
    }
    Mixxx.ControlProxy {
        id: showSpinniesControl

        group: "[Skin]"
        key: "show_spinnies"
    }
    Mixxx.ControlProxy {
        id: showCoverArtControl

        group: "[Skin]"
        key: "show_coverart"
    }
    Mixxx.ControlProxy {
        id: selectBigSpinnyOrCoverControl

        group: "[Skin]"
        key: "select_big_spinny_or_cover"
    }
    Mixxx.ControlProxy {
        id: eqKnobsControl

        group: "[Skin]"
        key: "show_eq_knobs"
    }
    Mixxx.ControlProxy {
        id: eqKillsControl

        group: "[Skin]"
        key: "show_eq_kill_buttons"
    }
    Mixxx.ControlProxy {
        id: crossfaderControl

        group: "[Skin]"
        key: "show_xfader"
    }
    Mixxx.ControlProxy {
        id: mainHeadMixerControl

        group: "[Skin]"
        key: "show_main_head_mixer"
    }
    Mixxx.ControlProxy {
        id: equalWaveformHeightsControl

        group: "[Skin]"
        key: "equal_4deck_waveforms"
    }
    Mixxx.ControlProxy {
        id: hotcueTimingShiftControl

        group: "[Skin]"
        key: "timing_shift_buttons"
    }
    Mixxx.ControlProxy {
        id: show4EffectUnitsControl

        group: "[Skin]"
        key: "show_4effectunits"
    }
    Mixxx.ControlProxy {
        id: showSuperKnobsControl

        group: "[Skin]"
        key: "show_superknobs"
    }
    Mixxx.ControlProxy {
        id: showPreviewDecksControl

        group: "[Skin]"
        key: "show_preview_decks"
    }
    Mixxx.ControlProxy {
        id: showLibraryCoverArtControl

        group: "[Skin]"
        key: "show_library_coverart"
    }
    Mixxx.ControlProxy {
        id: samplerRowsControl

        group: "[LateNight]"
        key: "sampler_rows"
    }
    Mixxx.ControlProxy {
        id: expandSamplers14Control

        group: "[LateNight]"
        key: "expand_samplers_1-4"
    }
    Mixxx.ControlProxy {
        id: expandSamplers18Control

        group: "[LateNight]"
        key: "expand_samplers_1-8"
    }
    Mixxx.ControlProxy {
        id: expandSamplers916Control

        group: "[LateNight]"
        key: "expand_samplers_9-16"
    }
    Mixxx.ControlProxy {
        id: showSamplerFxControl

        group: "[Skin]"
        key: "show_sampler_fx"
    }
    Mixxx.ControlProxy {
        id: loadSamplerBankControl

        group: "[Sampler]"
        key: "LoadSamplerBank"
    }
    Mixxx.ControlProxy {
        id: saveSamplerBankControl

        group: "[Sampler]"
        key: "SaveSamplerBank"
    }
    Mixxx.ControlProxy {
        id: latencyUsageControl

        group: "[App]"
        key: "audio_latency_usage"
    }
    Mixxx.ControlProxy {
        id: latencyOverloadControl

        group: "[App]"
        key: "audio_latency_overload"
    }
    Mixxx.ControlProxy {
        id: recordingStatusControl

        group: "[Recording]"
        key: "status"

    }
    Mixxx.ControlProxy {
        id: recordingToggleControl

        group: "[Recording]"
        key: "toggle_recording"
    }
    Mixxx.ControlProxy {
        id: broadcastEnabledControl

        group: "[Shoutcast]"
        key: "enabled"
    }
    Mixxx.ControlProxy {
        id: broadcastStatusControl

        group: "[Shoutcast]"
        key: "status"
    }
    Mixxx.ControlProxy {
        id: autoDjControl

        group: "[AutoDJ]"
        key: "enabled"
    }
    Timer {
        id: clockTimer

        interval: 1000
        repeat: true
        running: true
        triggeredOnStart: true

        onTriggered: {
            clockLabel.text = root.formatTime(new Date());
        }
    }
    SequentialAnimation {
        id: warningPulse

        loops: Animation.Infinite
        running: latencyOverloadControl.value > 0 || broadcastStatusControl.value === 1.0 || broadcastStatusControl.value === 3.0 || broadcastStatusControl.value === 4.0

        PropertyAnimation {
            duration: 450
            property: "opacity"
            target: pulseDriver
            to: 0.35
        }
        PropertyAnimation {
            duration: 450
            property: "opacity"
            target: pulseDriver
            to: 1.0
        }
    }
    QtObject {
        id: pulseDriver

        property real opacity: 1.0
    }
    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 2
        anchors.rightMargin: 2
        spacing: 4

        RowLayout {
            Layout.alignment: Qt.AlignVCenter
            spacing: -2

            LateNightToolbarButton {
                id: maximizeLibraryButton

                buttonWidth: 80
                text: "BIG LIBRARY"

                onActivated: {
                    showMaximizedLibraryControl.value = checked ? 1.0 : 0.0;
                }
            }
            LateNightToolbarDropButton {
                popup: librarySettingsPopup

                onClicked: {
                    root.openPopupForButton(popup, this);
                }
            }
        }
        RowLayout {
            Layout.alignment: Qt.AlignVCenter
            spacing: 4
            visible: !maximizeLibraryButton.checked

            RowLayout {
                Layout.alignment: Qt.AlignVCenter
                spacing: -2

                LateNightToolbarButton {
                    id: showWaveformsButton

                    buttonWidth: 80
                    text: "WAVEFORMS"

                    onActivated: {
                        root.setShowWaveforms(checked);
                    }
                }
                LateNightToolbarDropButton {
                    popup: waveformSettingsPopup

                    onClicked: {
                        root.openPopupForButton(popup, this);
                    }
                }
            }
            RowLayout {
                Layout.alignment: Qt.AlignVCenter
                spacing: -2
                visible: root.show4decksAvailable

                LateNightToolbarButton {
                    id: show4DecksButton

                    buttonWidth: 52
                    text: "4 DECKS"

                    onActivated: {
                        root.setShow4Decks(checked);
                    }
                }
                LateNightToolbarDropButton {
                    popup: deckSettingsPopup

                    onClicked: {
                        root.openPopupForButton(popup, this);
                    }
                }
            }
            RowLayout {
                Layout.alignment: Qt.AlignVCenter
                spacing: -2

                LateNightToolbarButton {
                    id: showMixerButton

                    buttonWidth: 48
                    text: "MIXER"

                    onActivated: {
                        root.setShowMixer(checked);
                    }
                }
                LateNightToolbarDropButton {
                    popup: mixerSettingsPopup

                    onClicked: {
                        root.openPopupForButton(popup, this);
                    }
                }
            }
            RowLayout {
                Layout.alignment: Qt.AlignVCenter
                spacing: -2

                LateNightToolbarButton {
                    id: showEffectsButton

                    buttonWidth: 59
                    text: "EFFECTS"

                    onActivated: {
                        showEffectRackControl.value = checked ? 1.0 : 0.0;
                    }
                }
                LateNightToolbarDropButton {
                    popup: effectSettingsPopup

                    onClicked: {
                        root.openPopupForButton(popup, this);
                    }
                }
            }
            RowLayout {
                Layout.alignment: Qt.AlignVCenter
                spacing: -2

                LateNightToolbarButton {
                    id: showSamplersButton

                    buttonWidth: 71
                    text: "SAMPLERS"

                    onActivated: {
                        showSamplersControl.value = checked ? 1.0 : 0.0;
                    }
                }
                LateNightToolbarDropButton {
                    popup: samplerSettingsPopup

                    onClicked: {
                        root.openPopupForButton(popup, this);
                    }
                }
            }
            RowLayout {
                Layout.alignment: Qt.AlignVCenter
                spacing: -2

                LateNightToolbarButton {
                    id: showMicAuxButton

                    buttonWidth: 61
                    text: "MIC/AUX"

                    onActivated: {
                        showMicAuxControl.value = checked ? 1.0 : 0.0;
                    }
                }
                LateNightToolbarDropButton {
                    popup: micAuxSettingsPopup

                    onClicked: {
                        root.openPopupForButton(popup, this);
                    }
                }
            }
        }
        RowLayout {
            Layout.alignment: Qt.AlignVCenter
            spacing: 4
            visible: maximizeLibraryButton.checked

            RowLayout {
                Layout.alignment: Qt.AlignVCenter
                spacing: -2

                LateNightToolbarButton {
                    id: maxLibraryDecksButton

                    buttonWidth: 80
                    text: "DECKS"

                    onActivated: {
                        root.setShowMaximizedDecks(checked);
                    }
                }
                LateNightToolbarDropButton {
                    popup: deckSettingsPopup

                    onClicked: {
                        root.openPopupForButton(popup, this);
                    }
                }
            }
            RowLayout {
                Layout.alignment: Qt.AlignVCenter
                spacing: -2
                visible: root.show4decksAvailable

                LateNightToolbarButton {
                    id: maximizedShow4DecksButton

                    buttonWidth: 52
                    checked: show4DecksButton.checked
                    text: "4 DECKS"

                    onActivated: {
                        root.setShow4Decks(checked);
                    }
                }
                LateNightToolbarDropButton {
                    popup: deckSettingsPopup

                    onClicked: {
                        root.openPopupForButton(popup, this);
                    }
                }
            }
        }
        Item {
            Layout.fillWidth: true
            Layout.minimumWidth: 0
        }
        AutoDjIndicator {
            active: autoDjControl.value > 0
        }
        ClockIndicator {
            id: clockLabel
        }
        Item {
            Layout.preferredWidth: 7
        }
        LatencyIndicator {
            overload: latencyOverloadControl.value > 0
            overloadOpacity: pulseDriver.opacity
            usage: latencyUsageControl.initialized ? latencyUsageControl.value : 0.0
        }
        Item {
            Layout.preferredWidth: 7
        }
        BatteryIndicator {
            visible: Mixxx.Battery.isBatteryAvailable
        }
        Item {
            Layout.preferredWidth: 9
        }
        RecordingIndicator {
            durationText: Mixxx.Recording.durationText
            pulseOpacity: pulseDriver.opacity
            status: recordingStatusControl.value

            onClicked: {
                recordingToggleControl.trigger();
            }
        }
        BroadcastIndicator {
            pulseOpacity: pulseDriver.opacity
            status: broadcastStatusControl.value

            onClicked: {
                broadcastEnabledControl.value = broadcastEnabledControl.value > 0 ? 0.0 : 1.0;
            }
        }
        Image {
            Layout.alignment: Qt.AlignVCenter
            Layout.preferredHeight: 24
            Layout.preferredWidth: 74
            fillMode: Image.PreserveAspectFit
            source: LateNightTheme.legacyAsset("style", "mixxx_logo_small.svg")
        }
    }
    ToolbarSettingsPopup {
        id: deckSettingsPopup

        width: 205

        ColumnLayout {
            Layout.fillWidth: true
            Layout.bottomMargin: 7
            Layout.leftMargin: 5
            Layout.rightMargin: 5
            Layout.topMargin: 2
            spacing: 0

            RowLayout {
                Layout.fillWidth: true
                Layout.preferredHeight: 22
                spacing: 5

                Text {
                    color: LateNightTheme.primaryDeckTextColor
                    elide: Text.ElideRight
                    font.family: "Open Sans"
                    font.pixelSize: 17
                    text: "Decks"
                }

                Item {
                    Layout.preferredWidth: 10
                }

                ToolbarMenuInlineChoice {
                    checked: !show4DecksButton.checked
                    text: "2"

                    onClicked: {
                        root.setShow4Decks(false);
                    }
                }
                ToolbarMenuInlineChoice {
                    checked: show4DecksButton.checked
                    text: "4"

                    onClicked: {
                        root.setShow4Decks(true);
                    }
                }
            }
            RowLayout {
                Layout.fillWidth: true
                Layout.preferredHeight: 18
                spacing: 5

                Text {
                    color: LateNightTheme.textColor
                    font.family: "Open Sans"
                    font.pixelSize: 12
                    text: "Deck Size:"
                }
                Item {
                    id: hideMixerBtn
                    Layout.preferredHeight: 18
                    Layout.fillWidth: true
                    visible: showMixerButton.checked

                    Rectangle {
                        anchors.fill: parent
                        color: hideMixerMouseArea.containsMouse ? (LateNightTheme.isPaleMoon ? "#2c454f" : "#5e4507") : "transparent"
                        radius: 1
                    }

                    Text {
                        anchors.left: parent.left
                        anchors.leftMargin: 4
                        anchors.verticalCenter: parent.verticalCenter
                        color: hideMixerMouseArea.containsMouse ? "#ffffff" : LateNightTheme.textColor
                        font.family: "Open Sans"
                        font.pixelSize: 12
                        text: "hide mixer to select"
                    }

                    MouseArea {
                        id: hideMixerMouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            root.setShowMixer(false);
                        }
                    }
                }
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 4
                    visible: !showMixerButton.checked

                    ToolbarMenuInlineChoice {
                        checked: deckSizeControl.value === 2.0 || showFullDeckControl.value > 0
                        text: "Full"
                        widthOverride: 40

                        onClicked: {
                            root.setDeckSize(2.0);
                        }
                    }
                    ToolbarMenuInlineChoice {
                        checked: deckSizeControl.value === 1.0 || showCompactDeckControl.value > 0
                        text: "Compact"
                        widthOverride: 66

                        onClicked: {
                            root.setDeckSize(1.0);
                        }
                    }
                    ToolbarMenuInlineChoice {
                        checked: deckSizeControl.value === 0.0 || showMiniDeckControl.value > 0
                        text: "Mini"
                        widthOverride: 40

                        onClicked: {
                            root.setDeckSize(0.0);
                        }
                    }
                }
            }
            RowLayout {
                Layout.fillWidth: true
                spacing: 5

                ToolbarMenuToggle {
                    Layout.fillWidth: false
                    control: showHotcuesControl
                    text: "Hotcues"
                }
                ToolbarMenuInlineChoice {
                    checked: showHotcuesControl.value > 0 && show8HotcuesControl.value <= 0
                    enabled: showHotcuesControl.value > 0
                    text: "4"

                    onClicked: {
                        showHotcuesControl.value = 1.0;
                        show8HotcuesControl.value = 0.0;
                    }
                }
                ToolbarMenuInlineChoice {
                    checked: showHotcuesControl.value > 0 && show8HotcuesControl.value > 0
                    enabled: showHotcuesControl.value > 0
                    text: "8"

                    onClicked: {
                        showHotcuesControl.value = 1.0;
                        show8HotcuesControl.value = 1.0;
                    }
                }
            }
            ToolbarMenuToggle {
                control: showIntroOutroCuesControl
                text: "Intro & Outro Cues"
            }
            ToolbarMenuToggle {
                control: showLoopControlsControl
                text: "Loop Controls"
            }
            ToolbarMenuToggle {
                control: showBeatjumpControlsControl
                text: "Beatjump Controls"
            }
            ToolbarMenuToggle {
                control: showRateControlsControl
                text: "Rate Controls"
            }
            ToolbarMenuToggle {
                control: showRateControlButtonsControl
                enabled: showRateControlsControl.value > 0
                indent: 14
                text: "Rate Adjust Buttons"
            }
            ToolbarMenuToggle {
                control: showKeyControlsControl
                text: "Key Controls"
            }
            ToolbarMenuToggle {
                control: showVinylControlsControl
                text: "Vinyl Control"
            }
            ToolbarMenuToggle {
                control: showSpinniesControl
                text: "Spinny"
            }
            ToolbarMenuToggle {
                control: showCoverArtControl
                text: "Cover Art"
            }
            ToolbarMenuToggle {
                control: selectBigSpinnyOrCoverControl
                enabled: showCoverArtControl.value > 0
                indent: 14
                text: "Big Spinny/Cover Art"
            }
        }
    }
    ToolbarSettingsPopup {
        id: mixerSettingsPopup

        ToolbarMenuSectionToggle {
            title: "Mixer"
            control: showMixerControl

            ToolbarMenuToggle {
                control: mainHeadMixerControl
                enabled: showMixerControl.value > 0
                text: "Main / Headphone Mixer"
            }
            ToolbarMenuToggle {
                control: eqKnobsControl
                enabled: showMixerControl.value > 0
                text: "EQ Knobs"
            }
            ToolbarMenuToggle {
                control: eqKillsControl
                enabled: showMixerControl.value > 0 && eqKnobsControl.value > 0
                indent: 14
                text: "EQ Kill Buttons"
            }
            ToolbarMenuToggle {
                control: crossfaderControl
                enabled: showMixerControl.value > 0
                text: "Crossfader"
            }
        }
    }
    ToolbarSettingsPopup {
        id: waveformSettingsPopup

        ToolbarMenuSectionToggle {
            title: "Waveforms"
            control: showWaveformsControl

            ToolbarMenuToggle {
                control: hotcueTimingShiftControl
                enabled: showWaveformsControl.value > 0
                text: "Hotcue Shift Buttons"
            }
            ToolbarMenuToggle {
                control: equalWaveformHeightsControl
                enabled: showWaveformsControl.value > 0
                text: "Enforce equal heights"
            }
        }
    }
    ToolbarSettingsPopup {
        id: effectSettingsPopup

        width: 230

        ColumnLayout {
            Layout.fillWidth: true
            Layout.bottomMargin: 7
            Layout.leftMargin: 5
            Layout.rightMargin: 5
            Layout.topMargin: 2
            spacing: 0

            Item {
                id: effectUnitsHeader
                property bool checked: showEffectRackControl.value > 0

                Layout.fillWidth: true
                implicitHeight: 22

                MouseArea {
                    anchors.fill: parent

                    onClicked: {
                        showEffectRackControl.value = effectUnitsHeader.checked ? 0.0 : 1.0;
                    }
                }

                RowLayout {
                    anchors.fill: parent
                    spacing: 5

                    Image {
                        Layout.preferredHeight: 18
                        Layout.preferredWidth: 18
                        fillMode: Image.PreserveAspectFit
                        source: effectUnitsHeader.checked ? (LateNightTheme.isPaleMoon ? LateNightTheme.legacyAsset("buttons", "btn__lib_checkmark_ivory.svg") : LateNightTheme.legacyAsset("buttons", "btn__lib_checkmark_grey.svg")) : LateNightTheme.legacyAsset("buttons", "btn__menu_checkbox.svg")
                    }

                    Text {
                        color: LateNightTheme.primaryDeckTextColor
                        elide: Text.ElideRight
                        font.family: "Open Sans"
                        font.pixelSize: 17
                        text: "Effect Units"
                    }

                    Item { Layout.fillWidth: true } // spacer

                    ToolbarMenuInlineChoice {
                        checked: show4EffectUnitsControl.value === 0.0
                        enabled: showEffectRackControl.value > 0
                        text: "2"

                        onClicked: {
                            show4EffectUnitsControl.value = 0.0;
                        }
                    }

                    ToolbarMenuInlineChoice {
                        checked: show4EffectUnitsControl.value === 1.0
                        enabled: showEffectRackControl.value > 0
                        text: "4"

                        onClicked: {
                            show4EffectUnitsControl.value = 1.0;
                        }
                    }
                }
            }

            ToolbarMenuToggle {
                control: showSuperKnobsControl
                enabled: showEffectRackControl.value > 0
                text: "Super Knobs"
            }
        }
    }
    ToolbarSettingsPopup {
        id: samplerSettingsPopup

        width: 250

        ColumnLayout {
            Layout.fillWidth: true
            Layout.bottomMargin: 7
            Layout.leftMargin: 5
            Layout.rightMargin: 5
            Layout.topMargin: 2
            spacing: 0

            Item {
                id: samplersHeader
                property bool checked: showSamplersControl.value > 0

                Layout.fillWidth: true
                implicitHeight: 22

                MouseArea {
                    anchors.fill: parent

                    onClicked: {
                        showSamplersControl.value = samplersHeader.checked ? 0.0 : 1.0;
                    }
                }

                RowLayout {
                    anchors.fill: parent
                    spacing: 5

                    Image {
                        Layout.preferredHeight: 18
                        Layout.preferredWidth: 18
                        fillMode: Image.PreserveAspectFit
                        source: samplersHeader.checked ? (LateNightTheme.isPaleMoon ? LateNightTheme.legacyAsset("buttons", "btn__lib_checkmark_ivory.svg") : LateNightTheme.legacyAsset("buttons", "btn__lib_checkmark_grey.svg")) : LateNightTheme.legacyAsset("buttons", "btn__menu_checkbox.svg")
                    }

                    Text {
                        color: LateNightTheme.primaryDeckTextColor
                        elide: Text.ElideRight
                        font.family: "Open Sans"
                        font.pixelSize: 17
                        text: "Samplers"
                    }

                    Item { Layout.fillWidth: true } // spacer

                    ToolbarMenuInlineChoice {
                        checked: samplerRowsControl.value === 0.0
                        enabled: showSamplersControl.value > 0
                        text: "4"

                        onClicked: {
                            samplerRowsControl.value = 0.0;
                        }
                    }

                    ToolbarMenuInlineChoice {
                        checked: samplerRowsControl.value === 1.0
                        enabled: showSamplersControl.value > 0
                        text: "8"

                        onClicked: {
                            samplerRowsControl.value = 1.0;
                        }
                    }

                    ToolbarMenuInlineChoice {
                        checked: samplerRowsControl.value === 2.0
                        enabled: showSamplersControl.value > 0
                        text: "16"
                        widthOverride: 32

                        onClicked: {
                            samplerRowsControl.value = 2.0;
                        }
                    }

                    ToolbarMenuInlineChoice {
                        checked: samplerRowsControl.value === 4.0
                        enabled: showSamplersControl.value > 0
                        text: "64"
                        widthOverride: 32

                        onClicked: {
                            samplerRowsControl.value = 4.0;
                        }
                    }
                }
            }

            ToolbarMenuToggle {
                control: showSamplerFxControl
                enabled: showSamplersControl.value > 0
                text: "Fx controls"
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: 22
                Layout.preferredHeight: 18
                spacing: 3
                enabled: showSamplersControl.value > 0
                opacity: enabled ? 1.0 : 0.45

                Item {
                    id: loadBankBtn
                    Layout.preferredHeight: 18
                    Layout.preferredWidth: loadBankText.implicitWidth + 8

                    Rectangle {
                        anchors.fill: parent
                        color: loadBankMouseArea.containsMouse ? (LateNightTheme.isPaleMoon ? "#2c454f" : "#5e4507") : "transparent"
                        radius: 1
                    }

                    Text {
                        id: loadBankText
                        anchors.centerIn: parent
                        color: loadBankMouseArea.containsMouse ? "#ffffff" : LateNightTheme.textColor
                        font.family: "Open Sans"
                        font.pixelSize: 12
                        text: "Load"
                    }

                    MouseArea {
                        id: loadBankMouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            loadSamplerBankControl.value = 1.0;
                        }
                    }
                }

                Text {
                    color: LateNightTheme.textColor
                    font.family: "Open Sans"
                    font.pixelSize: 12
                    text: "/"
                }

                Item {
                    id: saveBankBtn
                    Layout.preferredHeight: 18
                    Layout.preferredWidth: saveBankText.implicitWidth + 8

                    Rectangle {
                        anchors.fill: parent
                        color: saveBankMouseArea.containsMouse ? (LateNightTheme.isPaleMoon ? "#2c454f" : "#5e4507") : "transparent"
                        radius: 1
                    }

                    Text {
                        id: saveBankText
                        anchors.centerIn: parent
                        color: saveBankMouseArea.containsMouse ? "#ffffff" : LateNightTheme.textColor
                        font.family: "Open Sans"
                        font.pixelSize: 12
                        text: "Save"
                    }

                    MouseArea {
                        id: saveBankMouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            saveSamplerBankControl.value = 1.0;
                        }
                    }
                }

                Text {
                    color: LateNightTheme.textColor
                    font.family: "Open Sans"
                    font.pixelSize: 12
                    text: "Sampler Bank"
                    Layout.fillWidth: true
                }
            }
        }
    }
    ToolbarSettingsPopup {
        id: librarySettingsPopup

        width: 140

        ToolbarMenuSection {
            title: "Library"

            ToolbarMenuToggle {
                control: showPreviewDecksControl
                text: "Preview Deck"
            }
            ToolbarMenuToggle {
                control: showLibraryCoverArtControl
                text: "Cover Art"
            }
        }
    }
    ToolbarSettingsPopup {
        id: micAuxSettingsPopup

        width: 140

        ToolbarMenuSection {
            title: "Mic/Aux"

            ToolbarMenuToggle {
                control: showMicAuxControl
                text: "Microphones"
            }
        }
    }

    component AutoDjIndicator: Image {
        required property bool active

        Layout.alignment: Qt.AlignVCenter
        Layout.preferredHeight: 22
        Layout.preferredWidth: active ? 30 : 0
        fillMode: Image.PreserveAspectFit
        source: LateNightTheme.legacyAsset("style", "autodj_status.svg")
        visible: active
    }
    component BatteryIndicator: Shared.BatteryIcon {
        Layout.alignment: Qt.AlignVCenter
        Layout.preferredHeight: 24
        Layout.preferredWidth: 24
        chargedSource: LateNightTheme.legacyAsset("style/batt", "ic_battery_charged.svg")
        chargingSources: [LateNightTheme.legacyAsset("style/batt", "ic_battery_charging_0.svg"), LateNightTheme.legacyAsset("style/batt", "ic_battery_charging_1.svg"), LateNightTheme.legacyAsset("style/batt", "ic_battery_charging_2.svg"), LateNightTheme.legacyAsset("style/batt", "ic_battery_charging_3.svg"), LateNightTheme.legacyAsset("style/batt", "ic_battery_charging_4.svg"), LateNightTheme.legacyAsset("style/batt", "ic_battery_charging_5.svg"), LateNightTheme.legacyAsset("style/batt", "ic_battery_charging_6.svg"), LateNightTheme.legacyAsset("style/batt", "ic_battery_charging_7.svg")]
        dischargingSources: [LateNightTheme.legacyAsset("style/batt", "ic_battery_discharging_0.svg"), LateNightTheme.legacyAsset("style/batt", "ic_battery_discharging_1.svg"), LateNightTheme.legacyAsset("style/batt", "ic_battery_discharging_2.svg"), LateNightTheme.legacyAsset("style/batt", "ic_battery_discharging_3.svg"), LateNightTheme.legacyAsset("style/batt", "ic_battery_discharging_4.svg"), LateNightTheme.legacyAsset("style/batt", "ic_battery_discharging_5.svg"), LateNightTheme.legacyAsset("style/batt", "ic_battery_discharging_6.svg"), LateNightTheme.legacyAsset("style/batt", "ic_battery_discharging_7.svg")]
        fillMode: Image.PreserveAspectFit
    }
    component BroadcastIndicator: MouseArea {
        id: broadcastIndicator

        required property real pulseOpacity
        required property real status

        Layout.alignment: Qt.AlignVCenter
        Layout.preferredHeight: 20
        Layout.preferredWidth: 72

        LateNightToolbarButtonBackground {
            active: broadcastIndicator.status > 0
            fillColor: root.broadcastBackgroundColor(broadcastIndicator.status)
        }
        Image {
            anchors.fill: parent
            fillMode: Image.Stretch
            source: broadcastIndicator.status > 0 ? LateNightTheme.legacyAsset("buttons", "btn__broadcast_on.svg") : LateNightTheme.legacyAsset("buttons", "btn__broadcast_off.svg")
        }
        Text {
            anchors.fill: parent
            anchors.leftMargin: 15
            color: broadcastIndicator.status > 0 ? LateNightTheme.toolbarButtonActiveTextColor : LateNightTheme.toolbarButtonInactiveTextColor
            font.family: "Open Sans"
            font.pixelSize: 11
            font.styleName: "Bold"
            font.weight: Font.Bold
            horizontalAlignment: Text.AlignHCenter
            opacity: broadcastIndicator.status === 1.0 || broadcastIndicator.status === 3.0 || broadcastIndicator.status === 4.0 ? broadcastIndicator.pulseOpacity : 1.0
            renderType: Text.NativeRendering
            text: broadcastIndicator.status === 1.0 ? "..." : (broadcastIndicator.status === 3.0 || broadcastIndicator.status === 4.0 ? "ERROR" : "ON AIR")
            verticalAlignment: Text.AlignVCenter
        }
    }
    component ClockIndicator: Text {
        Layout.alignment: Qt.AlignVCenter
        Layout.preferredWidth: 82
        color: LateNightTheme.toolbarClockTextColor
        font.family: "Open Sans"
        font.pixelSize: 16
        horizontalAlignment: Text.AlignHCenter
        text: "--:-- --"
        verticalAlignment: Text.AlignVCenter
    }
    component LateNightToolbarButton: MouseArea {
        id: toolbarButton

        property int buttonWidth: 52
        property bool checked: false
        property string text: ""

        signal activated

        Layout.alignment: Qt.AlignVCenter
        Layout.preferredHeight: 20
        Layout.preferredWidth: buttonWidth
        cursorShape: Qt.PointingHandCursor

        onClicked: {
            checked = !checked;
            toolbarButton.activated();
        }

        LateNightToolbarButtonBackground {
            active: toolbarButton.checked
        }
        Text {
            anchors.fill: parent
            color: toolbarButton.checked ? LateNightTheme.toolbarButtonActiveTextColor : LateNightTheme.toolbarButtonInactiveTextColor
            elide: Text.ElideRight
            font.family: "Open Sans"
            font.pixelSize: 11
            font.styleName: "Bold"
            font.weight: Font.Bold
            horizontalAlignment: Text.AlignHCenter
            renderType: Text.NativeRendering
            text: toolbarButton.text
            verticalAlignment: Text.AlignVCenter
        }
    }
    component LateNightToolbarButtonBackground: Item {
        property bool active: false
        property color fillColor: active ? LateNightTheme.toolbarButtonActiveBackgroundColor : LateNightTheme.toolbarButtonInactiveBackgroundColor

        anchors.fill: parent

        Rectangle {
            anchors.fill: parent
            anchors.margins: 2
            color: parent.fillColor
        }
        BorderImage {
            anchors.fill: parent
            border.bottom: 2
            border.left: 2
            border.right: 2
            border.top: 2
            horizontalTileMode: BorderImage.Stretch
            source: parent.active ? LateNightTheme.legacyAsset("buttons", "btn_embedded_library_active.svg") : LateNightTheme.legacyAsset("buttons", "btn_embedded_library.svg")
            verticalTileMode: BorderImage.Stretch
        }
    }
    component LateNightToolbarDropButton: MouseArea {
        id: dropButton

        required property var popup

        Layout.alignment: Qt.AlignVCenter
        Layout.preferredHeight: 20
        Layout.preferredWidth: 18
        cursorShape: Qt.PointingHandCursor

        LateNightToolbarButtonBackground {
            active: dropButton.popup.visible
        }
        Image {
            anchors.centerIn: parent
            fillMode: Image.PreserveAspectFit
            height: 16
            source: LateNightTheme.assetToolbarDropdownIcon
            width: 12
        }
    }
    component LatencyIndicator: Item {
        id: latencyIndicator

        required property bool overload
        required property real overloadOpacity
        required property real usage

        Layout.alignment: Qt.AlignVCenter
        Layout.preferredHeight: 18
        Layout.preferredWidth: 61

        Column {
            anchors.centerIn: parent
            spacing: 1

            Rectangle {
                border.color: latencyIndicator.overload ? "#ffff00" : "#040404"
                border.width: 1
                color: "#040404"
                height: 7
                opacity: latencyIndicator.overload ? latencyIndicator.overloadOpacity : 1.0
                width: 61

                Image {
                    anchors.centerIn: parent
                    fillMode: Image.Stretch
                    height: 5
                    source: LateNightTheme.legacyAsset("style", "latency_bg.png")
                    width: 59
                }
                Item {
                    anchors.left: parent.left
                    anchors.leftMargin: 1
                    anchors.verticalCenter: parent.verticalCenter
                    clip: true
                    height: 5
                    width: Math.max(0, Math.min(1, latencyIndicator.usage) * 59)

                    Image {
                        fillMode: Image.Stretch
                        height: 5
                        source: LateNightTheme.legacyAsset("style", "latency_over.png")
                        width: 59
                    }
                }
            }
            Text {
                color: "#444444"
                font.family: "Open Sans"
                font.pixelSize: 8
                height: 10
                horizontalAlignment: Text.AlignHCenter
                text: "Buffer %"
                verticalAlignment: Text.AlignVCenter
                width: 61
            }
        }
    }
    component RecordingIndicator: MouseArea {
        id: recordingIndicator

        readonly property bool active: status > 0
        required property string durationText
        required property real pulseOpacity
        required property real status

        Layout.alignment: Qt.AlignVCenter
        Layout.preferredHeight: 20
        Layout.preferredWidth: 72
        cursorShape: Qt.PointingHandCursor

        LateNightToolbarButtonBackground {
            active: recordingIndicator.active
            fillColor: root.recordingBackgroundColor(recordingIndicator.status)
        }
        RowLayout {
            anchors.fill: parent
            spacing: 0

            Image {
                Layout.preferredHeight: 20
                Layout.preferredWidth: 15
                fillMode: Image.PreserveAspectFit
                opacity: recordingIndicator.status === 1.0 ? recordingIndicator.pulseOpacity : 1.0
                source: recordingIndicator.active ? LateNightTheme.legacyAsset("buttons", "btn__rec_dot_active.svg") : LateNightTheme.legacyAsset("buttons", "btn__rec_dot.svg")
            }
            Text {
                Layout.fillWidth: true
                color: recordingIndicator.active ? LateNightTheme.toolbarButtonActiveTextColor : LateNightTheme.toolbarButtonInactiveTextColor
                elide: Text.ElideRight
                font.family: "Open Sans"
                font.pixelSize: 16
                horizontalAlignment: Text.AlignHCenter
                text: recordingIndicator.status === 2.0 || recordingIndicator.status === 3.0 ? recordingIndicator.durationText : "REC"
                verticalAlignment: Text.AlignVCenter
            }
        }
    }
    component ToolbarMenuChoice: Item {
        property bool checked: false
        property string text: ""

        signal clicked

        Layout.fillWidth: true
        implicitHeight: 18
        implicitWidth: menuChoiceText.implicitWidth + 22
        opacity: enabled ? 1.0 : 0.45

        MouseArea {
            anchors.fill: parent

            onClicked: {
                if (parent.enabled) {
                    parent.clicked();
                }
            }
        }
        Image {
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            fillMode: Image.PreserveAspectFit
            height: 18
            source: parent.checked ? (LateNightTheme.isPaleMoon ? LateNightTheme.legacyAsset("buttons", "btn__lib_checkmark_ivory.svg") : LateNightTheme.legacyAsset("buttons", "btn__lib_checkmark_grey.svg")) : LateNightTheme.legacyAsset("buttons", "btn__menu_checkbox.svg")
            width: 18
        }
        Text {
            id: menuChoiceText

            anchors.left: parent.left
            anchors.leftMargin: 22
            anchors.verticalCenter: parent.verticalCenter
            color: "#d2d2d2"
            elide: Text.ElideRight
            font.family: "Open Sans"
            font.pixelSize: 13
            text: parent.text
            width: parent.width - anchors.leftMargin
        }
    }
    component ToolbarMenuInlineChoice: Item {
        id: inlineChoice
        property bool checked: false
        property string text: ""
        property int widthOverride: 28

        signal clicked

        Layout.preferredHeight: 18
        Layout.preferredWidth: widthOverride
        opacity: enabled ? 1.0 : 0.45

        Rectangle {
            anchors.fill: parent
            color: inlineMouseArea.containsMouse ? (LateNightTheme.isPaleMoon ? "#2c454f" : "#5e4507") : "transparent"
            radius: 1
        }

        MouseArea {
            id: inlineMouseArea
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor

            onClicked: {
                if (inlineChoice.enabled) {
                    inlineChoice.clicked();
                }
            }
        }
        Text {
            anchors.centerIn: parent
            color: inlineMouseArea.containsMouse ? "#ffffff" : (inlineChoice.checked ? LateNightTheme.primaryDeckTextColor : "#777777")
            font.family: "Open Sans"
            font.pixelSize: 17
            font.underline: inlineChoice.checked && !inlineMouseArea.containsMouse
            text: inlineChoice.text
        }
    }
    component ToolbarMenuSection: ColumnLayout {
        required property string title

        Layout.fillWidth: true
        Layout.bottomMargin: 7
        Layout.leftMargin: 5
        Layout.rightMargin: 5
        Layout.topMargin: 2
        spacing: 0

        Text {
            Layout.fillWidth: true
            color: LateNightTheme.primaryDeckTextColor
            elide: Text.ElideRight
            font.family: "Open Sans"
            font.pixelSize: 17
            text: parent.title
        }
    }
    component ToolbarMenuSectionToggle: ColumnLayout {
        id: sectionToggle
        required property string title
        required property var control

        Layout.fillWidth: true
        Layout.bottomMargin: 7
        Layout.leftMargin: 5
        Layout.rightMargin: 5
        Layout.topMargin: 2
        spacing: 0

        Item {
            id: headerToggle
            property bool checked: sectionToggle.control ? sectionToggle.control.value > 0 : false

            Layout.fillWidth: true
            implicitHeight: 22
            implicitWidth: headerText.implicitWidth + 22

            MouseArea {
                anchors.fill: parent

                onClicked: {
                    if (sectionToggle.enabled) {
                        sectionToggle.control.value = headerToggle.checked ? 0.0 : 1.0;
                    }
                }
            }
            Image {
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                fillMode: Image.PreserveAspectFit
                height: 18
                source: headerToggle.checked ? (LateNightTheme.isPaleMoon ? LateNightTheme.legacyAsset("buttons", "btn__lib_checkmark_ivory.svg") : LateNightTheme.legacyAsset("buttons", "btn__lib_checkmark_grey.svg")) : LateNightTheme.legacyAsset("buttons", "btn__menu_checkbox.svg")
                width: 18
            }
            Text {
                id: headerText

                anchors.left: parent.left
                anchors.leftMargin: 22
                anchors.verticalCenter: parent.verticalCenter
                color: LateNightTheme.primaryDeckTextColor
                elide: Text.ElideRight
                font.family: "Open Sans"
                font.pixelSize: 17
                text: sectionToggle.title
                width: parent.width - anchors.leftMargin
            }
        }
    }
    component ToolbarMenuToggle: Item {
        property bool checked: control ? control.value > 0 : false
        required property var control
        property int indent: 0
        property string text: ""

        Layout.fillWidth: true
        implicitHeight: 18
        implicitWidth: menuText.implicitWidth + indent + 22
        opacity: enabled ? 1.0 : 0.45

        MouseArea {
            anchors.fill: parent

            onClicked: {
                if (parent.enabled) {
                    parent.control.value = parent.checked ? 0.0 : 1.0;
                }
            }
        }
        Image {
            anchors.left: parent.left
            anchors.leftMargin: parent.indent
            anchors.verticalCenter: parent.verticalCenter
            fillMode: Image.PreserveAspectFit
            height: 18
            source: parent.checked ? (LateNightTheme.isPaleMoon ? LateNightTheme.legacyAsset("buttons", "btn__lib_checkmark_ivory.svg") : LateNightTheme.legacyAsset("buttons", "btn__lib_checkmark_grey.svg")) : LateNightTheme.legacyAsset("buttons", "btn__menu_checkbox.svg")
            width: 18
        }
        Text {
            id: menuText

            anchors.left: parent.left
            anchors.leftMargin: parent.indent + 22
            anchors.verticalCenter: parent.verticalCenter
            color: "#d2d2d2"
            elide: Text.ElideRight
            font.family: "Open Sans"
            font.pixelSize: 13
            text: parent.text
            width: parent.width - anchors.leftMargin
        }
    }
    component ToolbarSettingsPopup: Popup {
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        modal: false
        padding: 3
        width: 190

        background: Rectangle {
            border.color: "#585858"
            border.width: 1
            color: "#0f0f0f"
            radius: 2
        }
        contentItem: ColumnLayout {
            spacing: 0
        }
    }
}
