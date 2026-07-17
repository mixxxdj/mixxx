pragma ComponentBehavior: Bound

import "../LateNightTheme"
import "../../../qml" as Shared
import Mixxx 1.0 as Mixxx
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root

    enum DeckSize {
        Mini = 0,
        Compact = 1,
        Full = 2
    }

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
    readonly property string activeAppMenuSection: hoveredAppMenuSection.length > 0 ? hoveredAppMenuSection : selectedAppMenuSection
    readonly property string activeAppMenuSubmenu: hoveredAppMenuSubmenu.length > 0 ? hoveredAppMenuSubmenu : pinnedAppMenuSubmenu
    property string hoveredAppMenuSection: ""
    property string hoveredAppMenuSubmenu: ""
    property string pinnedAppMenuSubmenu: ""
    property string selectedAppMenuSection: "Options"
    property ToolbarSettingsPopup recentlyClosedPopup: null
    property MouseArea recentlyClosedPopupButton: null
    property double recentlyClosedPopupTimestamp: 0

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
        deckSizeControl.value = Toolbar.Compact;
        showPreviewDecksControl.value = 0.0;
        showLibraryCoverArtControl.value = 1.0;
        samplerRowsControl.value = 1.0;
        toolbarDefaultsControl.value = 1.0;
    }
    function closeSettingsPopups() {
        appMenuPopup.close();
        deckSettingsPopup.close();
        effectSettingsPopup.close();
        librarySettingsPopup.close();
        mixerSettingsPopup.close();
        samplerSettingsPopup.close();
        waveformSettingsPopup.close();
    }
    function clearAppMenuSubmenu() {
        hoveredAppMenuSubmenu = "";
        pinnedAppMenuSubmenu = "";
    }
    function positionPopupForButton(popup, button) {
        const anchorButton = button.popupAnchor ? button.popupAnchor : button;
        const mapped = anchorButton.mapToItem(root, 0, 0);
        popup.x = Math.max(0, Math.min(root.width - popup.width, mapped.x));
        popup.y = root.height + 2;
    }
    function openPopupForButton(popup, button) {
        const popupWasJustClosedByThisButton = recentlyClosedPopup === popup &&
                recentlyClosedPopupButton === button &&
                Date.now() - recentlyClosedPopupTimestamp < 250;
        if (popupWasJustClosedByThisButton) {
            recentlyClosedPopup = null;
            recentlyClosedPopupButton = null;
            recentlyClosedPopupTimestamp = 0;
            button.wasPopupOpenOnPress = false;
            return;
        }
        if (button.wasPopupOpenOnPress || (popup.visible && popup.anchorButton === button)) {
            popup.close();
            button.wasPopupOpenOnPress = false;
            return;
        }
        closeSettingsPopups();
        popup.anchorButton = button;
        positionPopupForButton(popup, button);
        popup.open();
        Qt.callLater(function() {
            if (popup.visible && popup.anchorButton === button) {
                positionPopupForButton(popup, button);
            }
        });
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
    function menuHoverColor(hovered, enabled) {
        return hovered && enabled ? LateNightTheme.toolbarMenuHoverColor : "transparent";
    }

    color: LateNightTheme.toolbarRootBackgroundColor
    height: 26

    Rectangle {
        anchors.bottom: parent.bottom
        color: LateNightTheme.toolbarBottomBorderColor
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
        id: loadSelectedTrackDeck1Control

        group: "[Channel1]"
        key: "LoadSelectedTrack"
    }
    Mixxx.ControlProxy {
        id: loadSelectedTrackDeck2Control

        group: "[Channel2]"
        key: "LoadSelectedTrack"
    }
    Mixxx.ControlProxy {
        id: loadSelectedTrackDeck3Control

        group: "[Channel3]"
        key: "LoadSelectedTrack"
    }
    Mixxx.ControlProxy {
        id: loadSelectedTrackDeck4Control

        group: "[Channel4]"
        key: "LoadSelectedTrack"
    }
    Mixxx.ControlProxy {
        id: vinylDeck1Control

        group: "[Channel1]"
        key: "vinylcontrol_enabled"
    }
    Mixxx.ControlProxy {
        id: vinylDeck2Control

        group: "[Channel2]"
        key: "vinylcontrol_enabled"
    }
    Mixxx.ControlProxy {
        id: vinylDeck3Control

        group: "[Channel3]"
        key: "vinylcontrol_enabled"
    }
    Mixxx.ControlProxy {
        id: vinylDeck4Control

        group: "[Channel4]"
        key: "vinylcontrol_enabled"
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

        LateNightToolbarMenuButton {
            id: appMenuButton

            popup: appMenuPopup

            onClicked: {
                root.openPopupForButton(appMenuPopup, this);
            }
        }
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
                popupAnchor: maximizeLibraryButton

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
                    popupAnchor: showWaveformsButton

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
                    popupAnchor: show4DecksButton

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
                    popupAnchor: showMixerButton

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
                    popupAnchor: showEffectsButton

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
                    popupAnchor: showSamplersButton

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

                    buttonWidth: 52
                    text: "DECKS"

                    onActivated: {
                        root.setShowMaximizedDecks(checked);
                    }
                }
                LateNightToolbarDropButton {
                    popup: deckSettingsPopup
                    popupAnchor: maxLibraryDecksButton

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
            source: LateNightTheme.lateNightAsset("style", "mixxx_logo_small.svg")
        }
    }
    ToolbarSettingsPopup {
        id: appMenuPopup

        minimumWidth: 260

        RowLayout {
            Layout.fillWidth: true
            spacing: 0

            ColumnLayout {
                Layout.alignment: Qt.AlignTop
                Layout.preferredWidth: 58
                spacing: 0

                ToolbarAppMenuTab {
                    selected: root.activeAppMenuSection === "File"
                    text: "File"

                    onTriggered: {
                        root.selectedAppMenuSection = "File";
                    }
                }
                ToolbarAppMenuTab {
                    selected: root.activeAppMenuSection === "Library"
                    text: "Library"

                    onTriggered: {
                        root.selectedAppMenuSection = "Library";
                    }
                }
                ToolbarAppMenuTab {
                    selected: root.activeAppMenuSection === "View"
                    text: "View"

                    onTriggered: {
                        root.selectedAppMenuSection = "View";
                    }
                }
                ToolbarAppMenuTab {
                    selected: root.activeAppMenuSection === "Options"
                    text: "Options"

                    onTriggered: {
                        root.selectedAppMenuSection = "Options";
                    }
                }
                ToolbarAppMenuTab {
                    selected: root.activeAppMenuSection === "Help"
                    text: "Help"

                    onTriggered: {
                        root.selectedAppMenuSection = "Help";
                    }
                }
            }
            Rectangle {
                Layout.fillHeight: true
                Layout.preferredWidth: 1
                color: LateNightTheme.toolbarPopupBorderColor
            }
            ColumnLayout {
                Layout.alignment: Qt.AlignTop
                Layout.minimumWidth: 180
                spacing: 0

                ToolbarAppMenuAction {
                    shortcut: "Ctrl+O"
                    text: "Load Track to Deck 1"
                    visible: root.activeAppMenuSection === "File"

                    onTriggered: {
                        loadSelectedTrackDeck1Control.trigger();
                    }
                }
                ToolbarAppMenuAction {
                    shortcut: "Ctrl+Shift+O"
                    text: "Load Track to Deck 2"
                    visible: root.activeAppMenuSection === "File"

                    onTriggered: {
                        loadSelectedTrackDeck2Control.trigger();
                    }
                }
                ToolbarAppMenuAction {
                    enabled: show4DecksButton.checked
                    text: "Load Track to Deck 3"
                    visible: root.activeAppMenuSection === "File"

                    onTriggered: {
                        loadSelectedTrackDeck3Control.trigger();
                    }
                }
                ToolbarAppMenuAction {
                    enabled: show4DecksButton.checked
                    text: "Load Track to Deck 4"
                    visible: root.activeAppMenuSection === "File"

                    onTriggered: {
                        loadSelectedTrackDeck4Control.trigger();
                    }
                }
                ToolbarAppMenuSeparator {
                    visible: root.activeAppMenuSection === "File"
                }
                ToolbarAppMenuAction {
                    enabled: false
                    shortcut: "Ctrl+Q"
                    text: "Exit"
                    visible: root.activeAppMenuSection === "File"
                }
                ToolbarAppMenuAction {
                    enabled: false
                    shortcut: "Ctrl+Shift+L"
                    text: "Rescan Library"
                    visible: root.activeAppMenuSection === "Library"
                }
                ToolbarAppMenuSeparator {
                    visible: root.activeAppMenuSection === "Library"
                }
                ToolbarAppMenuAction {
                    enabled: false
                    shortcut: "Ctrl+F"
                    text: "Search in Current View..."
                    visible: root.activeAppMenuSection === "Library"
                }
                ToolbarAppMenuAction {
                    enabled: false
                    shortcut: "Ctrl+Shift+F"
                    text: "Search in Tracks Library..."
                    visible: root.activeAppMenuSection === "Library"
                }
                ToolbarAppMenuSeparator {
                    visible: root.activeAppMenuSection === "Library"
                }
                ToolbarAppMenuAction {
                    enabled: false
                    shortcut: "Ctrl+N"
                    text: "Create New Playlist"
                    visible: root.activeAppMenuSection === "Library"
                }
                ToolbarAppMenuAction {
                    enabled: false
                    shortcut: "Ctrl+Shift+N"
                    text: "Create New Crate"
                    visible: root.activeAppMenuSection === "Library"
                }
                ToolbarAppMenuAction {
                    checkable: true
                    checked: showMicAuxControl.value > 0
                    shortcut: "Ctrl+2"
                    text: "Show Microphone Section"
                    visible: root.activeAppMenuSection === "View"

                    onTriggered: {
                        showMicAuxControl.value = showMicAuxControl.value > 0 ? 0.0 : 1.0;
                    }
                }
                ToolbarAppMenuAction {
                    checkable: true
                    checked: showVinylControlsControl.value > 0
                    shortcut: "Ctrl+3"
                    text: "Show Vinyl Control Section"
                    visible: root.activeAppMenuSection === "View"

                    onTriggered: {
                        showVinylControlsControl.value = showVinylControlsControl.value > 0 ? 0.0 : 1.0;
                    }
                }
                ToolbarAppMenuAction {
                    checkable: true
                    checked: showPreviewDecksControl.value > 0
                    enabled: false
                    shortcut: "Ctrl+4"
                    text: "Show Preview Deck"
                    visible: root.activeAppMenuSection === "View"
                }
                ToolbarAppMenuAction {
                    checkable: true
                    checked: showLibraryCoverArtControl.value > 0
                    shortcut: "Ctrl+6"
                    text: "Show Cover Art"
                    visible: root.activeAppMenuSection === "View"

                    onTriggered: {
                        showLibraryCoverArtControl.value = showLibraryCoverArtControl.value > 0 ? 0.0 : 1.0;
                    }
                }
                ToolbarAppMenuAction {
                    checkable: true
                    checked: maximizeLibraryButton.checked
                    shortcut: "Space"
                    text: "Maximize Library"
                    visible: root.activeAppMenuSection === "View"

                    onTriggered: {
                        showMaximizedLibraryControl.value = maximizeLibraryButton.checked ? 0.0 : 1.0;
                    }
                }
                ToolbarAppMenuSeparator {
                    visible: root.activeAppMenuSection === "View"
                }
                ToolbarAppMenuAction {
                    enabled: false
                    shortcut: "Ctrl+9"
                    text: "Show Auto DJ"
                    visible: root.activeAppMenuSection === "View"
                }
                ToolbarAppMenuAction {
                    enabled: false
                    shortcut: "F11"
                    text: "Full Screen"
                    visible: root.activeAppMenuSection === "View"
                }
                ToolbarAppMenuAction {
                    hasSubmenu: true
                    selected: root.activeAppMenuSubmenu === "Vinyl Control"
                    text: "Vinyl Control"
                    visible: root.activeAppMenuSection === "Options"

                    onHovered: {
                        root.hoveredAppMenuSubmenu = "Vinyl Control";
                    }
                    onTriggered: {
                        root.hoveredAppMenuSubmenu = "";
                        root.pinnedAppMenuSubmenu = root.pinnedAppMenuSubmenu === "Vinyl Control" ? "" : "Vinyl Control";
                    }
                }
                ToolbarAppMenuSeparator {
                    visible: root.activeAppMenuSection === "Options"
                }
                ToolbarAppMenuAction {
                    checkable: true
                    checked: recordingStatusControl.value > 0
                    shortcut: "Ctrl+R"
                    text: "Record Mix"
                    visible: root.activeAppMenuSection === "Options"

                    onHovered: {
                        root.hoveredAppMenuSubmenu = "";
                    }
                    onTriggered: {
                        recordingToggleControl.trigger();
                    }
                }
                ToolbarAppMenuAction {
                    checkable: true
                    checked: broadcastEnabledControl.value > 0
                    shortcut: "Ctrl+L"
                    text: "Enable Live Broadcasting"
                    visible: root.activeAppMenuSection === "Options"

                    onHovered: {
                        root.hoveredAppMenuSubmenu = "";
                    }
                    onTriggered: {
                        broadcastEnabledControl.value = broadcastEnabledControl.value > 0 ? 0.0 : 1.0;
                    }
                }
                ToolbarAppMenuSeparator {
                    visible: root.activeAppMenuSection === "Options"
                }
                ToolbarAppMenuAction {
                    enabled: false
                    checkable: true
                    checked: true
                    shortcut: "Ctrl+`"
                    text: "Enable Keyboard Shortcuts"
                    visible: root.activeAppMenuSection === "Options"

                    onHovered: {
                        root.hoveredAppMenuSubmenu = "";
                    }
                }
                ToolbarAppMenuSeparator {
                    visible: root.activeAppMenuSection === "Options"
                }
                ToolbarAppMenuAction {
                    shortcut: "Ctrl+P"
                    text: "Preferences"
                    visible: root.activeAppMenuSection === "Options"

                    onHovered: {
                        root.hoveredAppMenuSubmenu = "";
                    }
                    onTriggered: {
                        Mixxx.PreferencesDialog.show();
                        appMenuPopup.close();
                    }
                }
                ToolbarAppMenuAction {
                    enabled: false
                    text: "Community Support"
                    visible: root.activeAppMenuSection === "Help"
                }
                ToolbarAppMenuAction {
                    enabled: false
                    text: "User Manual"
                    visible: root.activeAppMenuSection === "Help"
                }
                ToolbarAppMenuAction {
                    enabled: false
                    text: "Keyboard Shortcuts"
                    visible: root.activeAppMenuSection === "Help"
                }
                ToolbarAppMenuAction {
                    enabled: false
                    text: "Settings directory"
                    visible: root.activeAppMenuSection === "Help"
                }
                ToolbarAppMenuAction {
                    enabled: false
                    text: "Translate This Application"
                    visible: root.activeAppMenuSection === "Help"
                }
                ToolbarAppMenuSeparator {
                    visible: root.activeAppMenuSection === "Help"
                }
                ToolbarAppMenuAction {
                    enabled: false
                    text: "About"
                    visible: root.activeAppMenuSection === "Help"
                }
            }
            Rectangle {
                Layout.fillHeight: true
                Layout.preferredWidth: 1
                color: LateNightTheme.toolbarPopupBorderColor
                visible: root.activeAppMenuSection === "Options" && root.activeAppMenuSubmenu === "Vinyl Control"
            }
            ColumnLayout {
                Layout.alignment: Qt.AlignTop
                Layout.minimumWidth: 180
                spacing: 0
                visible: root.activeAppMenuSection === "Options" && root.activeAppMenuSubmenu === "Vinyl Control"

                ToolbarAppMenuAction {
                    checkable: true
                    checked: vinylDeck1Control.value > 0
                    shortcut: "Ctrl+T"
                    text: "Enable Vinyl Control 1"

                    onTriggered: {
                        vinylDeck1Control.value = vinylDeck1Control.value > 0 ? 0.0 : 1.0;
                    }
                }
                ToolbarAppMenuAction {
                    checkable: true
                    checked: vinylDeck2Control.value > 0
                    shortcut: "Ctrl+Y"
                    text: "Enable Vinyl Control 2"

                    onTriggered: {
                        vinylDeck2Control.value = vinylDeck2Control.value > 0 ? 0.0 : 1.0;
                    }
                }
                ToolbarAppMenuAction {
                    checkable: true
                    checked: vinylDeck3Control.value > 0
                    shortcut: "Ctrl+U"
                    text: "Enable Vinyl Control 3"

                    onTriggered: {
                        vinylDeck3Control.value = vinylDeck3Control.value > 0 ? 0.0 : 1.0;
                    }
                }
                ToolbarAppMenuAction {
                    checkable: true
                    checked: vinylDeck4Control.value > 0
                    shortcut: "Ctrl+I"
                    text: "Enable Vinyl Control 4"

                    onTriggered: {
                        vinylDeck4Control.value = vinylDeck4Control.value > 0 ? 0.0 : 1.0;
                    }
                }
            }
        }
    }
    ToolbarSettingsPopup {
        id: deckSettingsPopup

        minimumWidth: 175

        ColumnLayout {
            Layout.fillWidth: true
            Layout.bottomMargin: 7
            Layout.leftMargin: 5
            Layout.rightMargin: 5
            Layout.topMargin: 2
            spacing: 0

            RowLayout {
                Layout.fillWidth: true
                Layout.minimumWidth: implicitWidth
                Layout.preferredHeight: 22
                spacing: 5

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
                Text {
                    color: LateNightTheme.toolbarMenuTextColor
                    font.family: "Open Sans"
                    font.pixelSize: 13
                    text: "decks"
                }
            }
            RowLayout {
                Layout.fillWidth: true
                Layout.minimumWidth: implicitWidth
                Layout.preferredHeight: 18
                spacing: 5

                Text {
                    color: LateNightTheme.toolbarMenuTextColor
                    font.family: "Open Sans"
                    font.pixelSize: 12
                    text: "Deck Size:"
                }
                Item {
                    id: hideMixerBtn
                    Layout.preferredHeight: 18
                    Layout.fillWidth: true
                    implicitWidth: hideMixerText.implicitWidth + 8
                    visible: showMixerButton.checked

                    Rectangle {
                        anchors.fill: parent
                        color: hideMixerMouseArea.containsMouse ? LateNightTheme.toolbarMenuHoverColor : "transparent"
                        radius: 1
                    }

                    Text {
                        id: hideMixerText
                        anchors.left: parent.left
                        anchors.leftMargin: 4
                        anchors.verticalCenter: parent.verticalCenter
                        color: hideMixerMouseArea.containsMouse ? LateNightTheme.toolbarMenuHoverTextColor : LateNightTheme.toolbarMenuTextColor
                        font.family: "Open Sans"
                        font.pixelSize: 12
                        text: "hide mixer to select"
                    }

                    MouseArea {
                        id: hideMixerMouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.ArrowCursor
                        onClicked: {
                            root.setShowMixer(false);
                        }
                    }
                }
                RowLayout {
                    Layout.fillWidth: true
                    Layout.minimumWidth: implicitWidth
                    spacing: 4
                    visible: !showMixerButton.checked

                    ToolbarMenuInlineChoice {
                        checked: deckSizeControl.value === Toolbar.Full
                        minimumWidth: 40
                        text: "Full"

                        onClicked: {
                            root.setDeckSize(Toolbar.Full);
                        }
                    }
                    ToolbarMenuInlineChoice {
                        checked: deckSizeControl.value === Toolbar.Compact
                        minimumWidth: 66
                        text: "Compact"

                        onClicked: {
                            root.setDeckSize(Toolbar.Compact);
                        }
                    }
                    ToolbarMenuInlineChoice {
                        checked: deckSizeControl.value === Toolbar.Mini
                        minimumWidth: 40
                        text: "Mini"

                        onClicked: {
                            root.setDeckSize(Toolbar.Mini);
                        }
                    }
                }
            }
            RowLayout {
                Layout.fillWidth: true
                Layout.minimumWidth: implicitWidth
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

        minimumWidth: 185

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
                Layout.minimumWidth: implicitWidth
                implicitHeight: 18
                implicitWidth: effectUnitsHeaderContent.implicitWidth

                Rectangle {
                    anchors.fill: parent
                    color: root.menuHoverColor(effectUnitsHeaderMouseArea.containsMouse, effectUnitsHeader.enabled)
                    radius: 1
                }

                MouseArea {
                    id: effectUnitsHeaderMouseArea
                    anchors.fill: parent
                    cursorShape: Qt.ArrowCursor
                    hoverEnabled: true

                    onClicked: {
                        showEffectRackControl.value = effectUnitsHeader.checked ? 0.0 : 1.0;
                    }
                }

                RowLayout {
                    id: effectUnitsHeaderContent

                    anchors.fill: parent
                    spacing: 5

                    Image {
                        Layout.leftMargin: 2
                        Layout.preferredHeight: 14
                        Layout.preferredWidth: 14
                        fillMode: Image.PreserveAspectFit
                        source: effectUnitsHeader.checked ? LateNightTheme.lateNightAsset("buttons", "btn__lib_checkmark_ivory.svg") : LateNightTheme.lateNightAsset("buttons", "btn__menu_checkbox.svg")
                    }

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
                    Text {
                        color: effectUnitsHeaderMouseArea.containsMouse ? LateNightTheme.toolbarMenuHoverTextColor : LateNightTheme.toolbarMenuTextColor
                        font.family: "Open Sans"
                        font.pixelSize: 13
                        text: "units"
                    }
                }
            }

            ToolbarMenuToggle {
                control: showSuperKnobsControl
                text: "Super Knobs"
            }
        }
    }
    ToolbarSettingsPopup {
        id: samplerSettingsPopup

        minimumWidth: 245

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
                Layout.minimumWidth: implicitWidth
                implicitHeight: 18
                implicitWidth: samplersHeaderContent.implicitWidth

                Rectangle {
                    anchors.fill: parent
                    color: root.menuHoverColor(samplersHeaderMouseArea.containsMouse, samplersHeader.enabled)
                    radius: 1
                }

                MouseArea {
                    id: samplersHeaderMouseArea
                    anchors.fill: parent
                    cursorShape: Qt.ArrowCursor
                    hoverEnabled: true

                    onClicked: {
                        showSamplersControl.value = samplersHeader.checked ? 0.0 : 1.0;
                    }
                }

                RowLayout {
                    id: samplersHeaderContent

                    anchors.fill: parent
                    spacing: 5

                    Image {
                        Layout.leftMargin: 2
                        Layout.preferredHeight: 14
                        Layout.preferredWidth: 14
                        fillMode: Image.PreserveAspectFit
                        source: samplersHeader.checked ? LateNightTheme.lateNightAsset("buttons", "btn__lib_checkmark_ivory.svg") : LateNightTheme.lateNightAsset("buttons", "btn__menu_checkbox.svg")
                    }

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
                        minimumWidth: 32
                        text: "16"

                        onClicked: {
                            samplerRowsControl.value = 2.0;
                        }
                    }

                    ToolbarMenuInlineChoice {
                        checked: samplerRowsControl.value === 4.0
                        enabled: showSamplersControl.value > 0
                        minimumWidth: 32
                        text: "64"

                        onClicked: {
                            samplerRowsControl.value = 4.0;
                        }
                    }
                    Text {
                        color: samplersHeaderMouseArea.containsMouse ? LateNightTheme.toolbarMenuHoverTextColor : LateNightTheme.toolbarMenuTextColor
                        font.family: "Open Sans"
                        font.pixelSize: 13
                        text: "sample decks"
                    }
                }
            }

            ToolbarMenuToggle {
                control: showSamplerFxControl
                text: "Fx controls"
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: 20
                Layout.minimumWidth: implicitWidth
                Layout.preferredHeight: 18
                spacing: 3
                enabled: showSamplerFxControl.value > 0
                opacity: enabled ? 1.0 : 0.45

                Item {
                    id: loadBankBtn
                    Layout.preferredHeight: 18
                    Layout.preferredWidth: loadBankText.implicitWidth + 8

                    Rectangle {
                        anchors.fill: parent
                        color: loadBankMouseArea.containsMouse ? LateNightTheme.toolbarMenuHoverColor : "transparent"
                        radius: 1
                    }

                    Text {
                        id: loadBankText
                        anchors.centerIn: parent
                        color: loadBankMouseArea.containsMouse ? LateNightTheme.toolbarMenuHoverTextColor : LateNightTheme.toolbarMenuTextColor
                        font.family: "Open Sans"
                        font.pixelSize: 12
                        text: "Load"
                    }

                    MouseArea {
                        id: loadBankMouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.ArrowCursor
                        onClicked: {
                            loadSamplerBankControl.value = 1.0;
                        }
                    }
                }

                Text {
                    color: LateNightTheme.toolbarMenuTextColor
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
                        color: saveBankMouseArea.containsMouse ? LateNightTheme.toolbarMenuHoverColor : "transparent"
                        radius: 1
                    }

                    Text {
                        id: saveBankText
                        anchors.centerIn: parent
                        color: saveBankMouseArea.containsMouse ? LateNightTheme.toolbarMenuHoverTextColor : LateNightTheme.toolbarMenuTextColor
                        font.family: "Open Sans"
                        font.pixelSize: 12
                        text: "Save"
                    }

                    MouseArea {
                        id: saveBankMouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.ArrowCursor
                        onClicked: {
                            saveSamplerBankControl.value = 1.0;
                        }
                    }
                }

                Text {
                    id: samplerBankText
                    color: LateNightTheme.toolbarMenuTextColor
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

        minimumWidth: 140

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
    component AutoDjIndicator: Image {
        required property bool active

        Layout.alignment: Qt.AlignVCenter
        Layout.preferredHeight: 22
        Layout.preferredWidth: active ? 30 : 0
        fillMode: Image.PreserveAspectFit
        source: LateNightTheme.lateNightAsset("style", "autodj_status.svg")
        visible: active
    }
    component BatteryIndicator: Shared.BatteryIcon {
        Layout.alignment: Qt.AlignVCenter
        Layout.preferredHeight: 24
        Layout.preferredWidth: 24
        chargedSource: LateNightTheme.lateNightAsset("style/batt", "ic_battery_charged.svg")
        chargingSources: [LateNightTheme.lateNightAsset("style/batt", "ic_battery_charging_0.svg"), LateNightTheme.lateNightAsset("style/batt", "ic_battery_charging_1.svg"), LateNightTheme.lateNightAsset("style/batt", "ic_battery_charging_2.svg"), LateNightTheme.lateNightAsset("style/batt", "ic_battery_charging_3.svg"), LateNightTheme.lateNightAsset("style/batt", "ic_battery_charging_4.svg"), LateNightTheme.lateNightAsset("style/batt", "ic_battery_charging_5.svg"), LateNightTheme.lateNightAsset("style/batt", "ic_battery_charging_6.svg"), LateNightTheme.lateNightAsset("style/batt", "ic_battery_charging_7.svg")]
        dischargingSources: [LateNightTheme.lateNightAsset("style/batt", "ic_battery_discharging_0.svg"), LateNightTheme.lateNightAsset("style/batt", "ic_battery_discharging_1.svg"), LateNightTheme.lateNightAsset("style/batt", "ic_battery_discharging_2.svg"), LateNightTheme.lateNightAsset("style/batt", "ic_battery_discharging_3.svg"), LateNightTheme.lateNightAsset("style/batt", "ic_battery_discharging_4.svg"), LateNightTheme.lateNightAsset("style/batt", "ic_battery_discharging_5.svg"), LateNightTheme.lateNightAsset("style/batt", "ic_battery_discharging_6.svg"), LateNightTheme.lateNightAsset("style/batt", "ic_battery_discharging_7.svg")]
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
            source: broadcastIndicator.status > 0 ? LateNightTheme.lateNightAsset("buttons", "btn__broadcast_on.svg") : LateNightTheme.lateNightAsset("buttons", "btn__broadcast_off.svg")
        }
        Text {
            anchors.fill: parent
            anchors.leftMargin: 15
            color: broadcastIndicator.status > 0 ? LateNightTheme.toolbarButtonActiveTextColor : LateNightTheme.toolbarButtonInactiveTextColor
            font {
                family: "Open Sans"
                pixelSize: 11
                styleName: "Bold"
                weight: Font.Bold
            }
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
    component LateNightToolbarMenuButton: MouseArea {
        id: menuButton

        required property ToolbarSettingsPopup popup
        property bool wasPopupOpenOnPress: false

        Layout.alignment: Qt.AlignVCenter
        Layout.preferredHeight: 20
        Layout.preferredWidth: 26
        cursorShape: Qt.ArrowCursor

        onPressed: {
            wasPopupOpenOnPress = popup.visible && popup.anchorButton === menuButton;
        }

        LateNightToolbarButtonBackground {
            active: menuButton.popup.visible
        }
        Image {
            anchors.centerIn: parent
            fillMode: Image.PreserveAspectFit
            height: 13
            source: LateNightTheme.assetToolbarMenuIcon
            width: 13
        }
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
        cursorShape: Qt.ArrowCursor

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
            font {
                family: "Open Sans"
                pixelSize: 11
                styleName: "Bold"
                weight: Font.Bold
            }
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
            source: parent.active ? LateNightTheme.lateNightAsset("buttons", "btn_embedded_library_active.svg") : LateNightTheme.lateNightAsset("buttons", "btn_embedded_library.svg")
            verticalTileMode: BorderImage.Stretch
        }
    }
    component LateNightToolbarDropButton: MouseArea {
        id: dropButton

        required property ToolbarSettingsPopup popup
        property MouseArea popupAnchor: null
        property bool wasPopupOpenOnPress: false

        Layout.alignment: Qt.AlignVCenter
        Layout.preferredHeight: 20
        Layout.preferredWidth: 18
        cursorShape: Qt.ArrowCursor

        onPressed: {
            wasPopupOpenOnPress = popup.visible && popup.anchorButton === dropButton;
        }

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
                border.color: latencyIndicator.overload ? LateNightTheme.toolbarLatencyOverloadColor : LateNightTheme.toolbarLatencyBorderColor
                border.width: 1
                color: LateNightTheme.toolbarLatencyBorderColor
                height: 7
                opacity: latencyIndicator.overload ? latencyIndicator.overloadOpacity : 1.0
                width: 61

                Image {
                    anchors.centerIn: parent
                    fillMode: Image.Stretch
                    height: 5
                    source: LateNightTheme.lateNightAsset("style", "latency_bg.png")
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
                        source: LateNightTheme.lateNightAsset("style", "latency_over.png")
                        width: 59
                    }
                }
            }
            Text {
                color: LateNightTheme.toolbarLatencyLabelColor
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
        cursorShape: Qt.ArrowCursor

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
                source: recordingIndicator.active ? LateNightTheme.lateNightAsset("buttons", "btn__rec_dot_active.svg") : LateNightTheme.lateNightAsset("buttons", "btn__rec_dot.svg")
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
    component ToolbarAppMenuTab: Item {
        id: appMenuTab

        property bool selected: false
        property string text: ""

        signal triggered

        Layout.fillWidth: true
        implicitHeight: 17
        implicitWidth: appMenuTabText.implicitWidth + 8

        Rectangle {
            anchors.fill: parent
            color: appMenuTabMouseArea.containsMouse || appMenuTab.selected ? LateNightTheme.toolbarMenuHoverColor : "transparent"
        }
        MouseArea {
            id: appMenuTabMouseArea

            anchors.fill: parent
            cursorShape: Qt.ArrowCursor
            hoverEnabled: true

            onEntered: {
                root.hoveredAppMenuSection = appMenuTab.text;
                root.clearAppMenuSubmenu();
            }
            onClicked: {
                root.hoveredAppMenuSection = "";
                root.clearAppMenuSubmenu();
                appMenuTab.triggered();
            }
        }
        Text {
            id: appMenuTabText

            anchors.left: parent.left
            anchors.leftMargin: 5
            anchors.verticalCenter: parent.verticalCenter
            color: appMenuTabMouseArea.containsMouse || appMenuTab.selected ? LateNightTheme.toolbarMenuHoverTextColor : LateNightTheme.toolbarMenuTextColor
            elide: Text.ElideRight
            font.family: "Open Sans"
            font.pixelSize: 12
            text: appMenuTab.text
            width: parent.width - anchors.leftMargin
        }
    }
    component ToolbarAppMenuSeparator: Rectangle {
        Layout.fillWidth: true
        Layout.leftMargin: 4
        Layout.rightMargin: 4
        Layout.preferredHeight: 1
        color: LateNightTheme.toolbarPopupBorderColor
    }
    component ToolbarAppMenuAction: Item {
        id: appMenuAction

        property bool checkable: false
        property bool checked: false
        property bool hasSubmenu: false
        property bool selected: false
        property string shortcut: ""
        property string text: ""

        signal hovered
        signal triggered

        Layout.fillWidth: true
        implicitHeight: 17
        implicitWidth: appMenuActionText.implicitWidth + appMenuShortcutText.implicitWidth + 42
        opacity: enabled ? 1.0 : 0.45

        Rectangle {
            anchors.fill: parent
            color: root.menuHoverColor(appMenuActionMouseArea.containsMouse || appMenuAction.selected, appMenuAction.enabled)
        }
        MouseArea {
            id: appMenuActionMouseArea

            anchors.fill: parent
            cursorShape: Qt.ArrowCursor
            hoverEnabled: true

            onEntered: {
                if (appMenuAction.enabled) {
                    appMenuAction.hovered();
                }
            }
            onClicked: {
                if (appMenuAction.enabled) {
                    appMenuAction.triggered();
                }
            }
        }
        Image {
            anchors.left: parent.left
            anchors.leftMargin: 3
            anchors.verticalCenter: parent.verticalCenter
            fillMode: Image.PreserveAspectFit
            height: 14
            source: appMenuAction.checked ? LateNightTheme.lateNightAsset("buttons", "btn__lib_checkmark_ivory.svg") : LateNightTheme.lateNightAsset("buttons", "btn__menu_checkbox.svg")
            visible: appMenuAction.checkable
            width: 14
        }
        Text {
            id: appMenuActionText

            anchors.left: parent.left
            anchors.leftMargin: appMenuAction.checkable ? 22 : 6
            anchors.right: appMenuShortcutText.left
            anchors.rightMargin: 8
            anchors.verticalCenter: parent.verticalCenter
            color: appMenuActionMouseArea.containsMouse || appMenuAction.selected ? LateNightTheme.toolbarMenuHoverTextColor : LateNightTheme.toolbarMenuTextColor
            elide: Text.ElideRight
            font.family: "Open Sans"
            font.pixelSize: 11
            text: appMenuAction.text
        }
        Text {
            id: appMenuShortcutText

            anchors.right: appMenuArrowText.left
            anchors.rightMargin: 8
            anchors.verticalCenter: parent.verticalCenter
            color: appMenuActionMouseArea.containsMouse || appMenuAction.selected ? LateNightTheme.toolbarMenuHoverTextColor : LateNightTheme.toolbarMenuTextColor
            font.family: "Open Sans"
            font.pixelSize: 10
            text: appMenuAction.shortcut
            visible: text.length > 0
        }
        Text {
            id: appMenuArrowText

            anchors.right: parent.right
            anchors.rightMargin: 6
            anchors.verticalCenter: parent.verticalCenter
            color: appMenuActionMouseArea.containsMouse || appMenuAction.selected ? LateNightTheme.toolbarMenuHoverTextColor : LateNightTheme.toolbarMenuTextColor
            font.family: "Open Sans"
            font.pixelSize: 11
            text: appMenuAction.hasSubmenu ? ">" : ""
            width: 8
        }
    }
    component ToolbarMenuInlineChoice: Item {
        id: inlineChoice
        property bool checked: false
        property string text: ""
        property int minimumWidth: 28

        signal clicked

        Layout.preferredHeight: 18
        Layout.preferredWidth: Math.max(minimumWidth, inlineChoiceText.implicitWidth + 8)
        opacity: enabled ? 1.0 : 0.45

        Rectangle {
            anchors.fill: parent
            color: root.menuHoverColor(inlineMouseArea.containsMouse, inlineChoice.enabled)
            radius: 1
        }

        MouseArea {
            id: inlineMouseArea
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.ArrowCursor

            onClicked: {
                if (inlineChoice.enabled) {
                    inlineChoice.clicked();
                }
            }
        }
        Text {
            id: inlineChoiceText

            anchors.centerIn: parent
            color: inlineMouseArea.containsMouse ? LateNightTheme.toolbarMenuHoverTextColor : (inlineChoice.checked ? LateNightTheme.toolbarMenuTextColor : LateNightTheme.toolbarMenuDisabledTextColor)
            font.family: "Open Sans"
            font.pixelSize: 13
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
            color: LateNightTheme.toolbarMenuTextColor
            elide: Text.ElideRight
            font.family: "Open Sans"
            font.pixelSize: 13
            text: parent.title
        }
    }
    component ToolbarMenuSectionToggle: ColumnLayout {
        id: sectionToggle
        required property string title
        required property Mixxx.ControlProxy control

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
            implicitHeight: 18
            implicitWidth: headerText.implicitWidth + 20

            Rectangle {
                anchors.fill: parent
                color: root.menuHoverColor(headerToggleMouseArea.containsMouse, headerToggle.enabled)
                radius: 1
            }

            MouseArea {
                id: headerToggleMouseArea
                anchors.fill: parent
                cursorShape: Qt.ArrowCursor
                hoverEnabled: true

                onClicked: {
                    if (sectionToggle.enabled) {
                        sectionToggle.control.value = headerToggle.checked ? 0.0 : 1.0;
                    }
                }
            }
            Image {
                anchors.left: parent.left
                anchors.leftMargin: 2
                anchors.verticalCenter: parent.verticalCenter
                fillMode: Image.PreserveAspectFit
                height: 14
                source: headerToggle.checked ? LateNightTheme.lateNightAsset("buttons", "btn__lib_checkmark_ivory.svg") : LateNightTheme.lateNightAsset("buttons", "btn__menu_checkbox.svg")
                width: 14
            }
            Text {
                id: headerText

                anchors.left: parent.left
                anchors.leftMargin: 20
                anchors.verticalCenter: parent.verticalCenter
                color: headerToggleMouseArea.containsMouse ? LateNightTheme.toolbarMenuHoverTextColor : LateNightTheme.toolbarMenuTextColor
                elide: Text.ElideRight
                font.family: "Open Sans"
                font.pixelSize: 13
                text: sectionToggle.title
                width: parent.width - anchors.leftMargin
            }
        }
    }
    component ToolbarMenuToggle: Item {
        id: menuToggle

        property bool checked: control ? control.value > 0 : false
        required property Mixxx.ControlProxy control
        property int indent: 0
        property string text: ""

        Layout.fillWidth: true
        implicitHeight: 18
        implicitWidth: menuText.implicitWidth + indent + 20
        opacity: enabled ? 1.0 : 0.45

        Rectangle {
            anchors.fill: parent
            color: root.menuHoverColor(menuToggleMouseArea.containsMouse, menuToggle.enabled)
            radius: 1
        }

        MouseArea {
            id: menuToggleMouseArea
            anchors.fill: parent
            cursorShape: Qt.ArrowCursor
            hoverEnabled: true

            onClicked: {
                if (menuToggle.enabled && menuToggle.control && menuToggle.control.initialized) {
                    menuToggle.control.value = menuToggle.checked ? 0.0 : 1.0;
                }
            }
        }
        Image {
            anchors.left: parent.left
            anchors.leftMargin: parent.indent + 2
            anchors.verticalCenter: parent.verticalCenter
            fillMode: Image.PreserveAspectFit
            height: 14
            source: parent.checked ? LateNightTheme.lateNightAsset("buttons", "btn__lib_checkmark_ivory.svg") : LateNightTheme.lateNightAsset("buttons", "btn__menu_checkbox.svg")
            width: 14
        }
        Text {
            id: menuText

            anchors.left: parent.left
            anchors.leftMargin: parent.indent + 20
            anchors.verticalCenter: parent.verticalCenter
            color: menuToggleMouseArea.containsMouse ? LateNightTheme.toolbarMenuHoverTextColor : LateNightTheme.toolbarMenuTextColor
            elide: Text.ElideRight
            font.family: "Open Sans"
            font.pixelSize: 13
            text: parent.text
            width: parent.width - anchors.leftMargin
        }
    }
    component ToolbarSettingsPopup: Popup {
        id: toolbarSettingsPopup

        property MouseArea anchorButton: null
        property int minimumWidth: 190

        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        modal: false
        padding: 3
        width: Math.min(root.width, Math.max(minimumWidth, contentColumn.implicitWidth + leftPadding + rightPadding))

        onClosed: {
            if (anchorButton) {
                root.recentlyClosedPopup = toolbarSettingsPopup;
                root.recentlyClosedPopupButton = anchorButton;
                root.recentlyClosedPopupTimestamp = Date.now();
            }
            anchorButton = null;
        }
        onWidthChanged: {
            if (visible && anchorButton) {
                root.positionPopupForButton(toolbarSettingsPopup, anchorButton);
            }
        }

        background: Rectangle {
            border.color: LateNightTheme.toolbarPopupBorderColor
            border.width: 1
            color: LateNightTheme.toolbarPopupBackgroundColor
            radius: 2
        }
        contentItem: ColumnLayout {
            id: contentColumn

            spacing: 0
        }
    }
}
