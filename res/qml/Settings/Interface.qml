import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Dialogs
import Mixxx 1.0 as Mixxx
import "." as Setting
import "../Deck" as DeckComponents
import "." as SettingComponents
import ".." as Skin
import "../Theme"

Category {
    id: root

    function load() {
        loadInterface();
        loadWaveform();
        loadDeck();
        errorMessage.text = "";
    }
    function loadDeck() {
        // Decks tab
        cueModeInput.currentIndex = Mixxx.Config.controlCueDefault;
        setIntroStartToMainCueInput.selected = Mixxx.Config.controlSetIntroStartAtMainCue ? "on" : "off";
        trackTimeDisplayInput.selected = trackTimeDisplayInput.options[Mixxx.Config.controlPositionDisplay];
        timeMode.currentIndex = Mixxx.Config.controlTimeFormat;
        doublePressLoadToCloneInput.selected = Mixxx.Config.controlCloneDeckOnLoadDoubleTap ? "on" : "off";
        trackLoadPointInput.currentIndex = Mixxx.Config.controlCueRecall;
        loadingTrackWhenPlayingInput.selected = loadingTrackWhenPlayingInput.options[Mixxx.Config.controlLoadWhenDeckPlaying];
        resetOnTrackLoadInput.selected = resetOnTrackLoadInput.options[Mixxx.Config.controlSpeedAutoReset];
        sliderRangeInput.currentIndex = sliderRangeInput.values.indexOf(Mixxx.Config.controlRateRange);
        if (sliderRangeInput.currentIndex === -1) {
            sliderRangeInput.values.push(Mixxx.Config.controlRateRange);
            sliderRangeInput.model.push(`${Mixxx.Config.controlRateRange}%`);
            sliderRangeInput.currentIndex = sliderRangeInput.model.length - 1;
        }
        syncModeInput.selected = syncModeInput.options[Mixxx.Config.bpmSyncLockAlgorithm];
        sliderOrientationInput.selected = Mixxx.Config.controlRateDir ? "down" : "up";
        keylockModeInput.selected = keylockModeInput.options[Mixxx.Config.controlKeylockMode];
        keyunlockModeInput.selected = keyunlockModeInput.options[Mixxx.Config.controlKeyunlockMode];
        pitchBendBehaviourInput.selected = pitchBendBehaviourInput.options[Mixxx.Config.controlPitchBendBehaviour];
        adjustmentButtonsTemporaryCoarseInput.value = Mixxx.Config.controlRateTempCoarse * 100;
        adjustmentButtonsTemporaryFineInput.value = Mixxx.Config.controlRateTempFine * 100;
        adjustmentButtonsPermanentCoarseInput.value = Mixxx.Config.controlRatePermCoarse * 100;
        adjustmentButtonsPermanentFineInput.value = Mixxx.Config.controlRatePermFine * 100;
        rampingSensitivityInput.value = Mixxx.Config.controlRateRampSensitivity;
        decksTab.dirty = false;
    }
    function loadInterface() {
        // Interface tab
        // skinInput.value =
        // colorInput.value =
        // layoutInput.value =
        tooltipsInput.selected = tooltipsInput.options[Mixxx.Config.libraryTooltips];
        disableScreensaverInput.selected = disableScreensaverInput.options[Mixxx.Config.libraryInhibitScreensaver];
        startFullscreenInput.selected = Mixxx.Config.configStartInFullscreenKey ? "on" : "off";
        autoHideMenuBarInput.selected = Mixxx.Config.libraryHideMenuBar ? "on" : "off";
        searchCompletionInput.selected = Mixxx.Config.libraryEnableSearchCompletions ? "on" : "off";
        searchHistoryKeyboardInput.selected = Mixxx.Config.libraryEnableSearchHistoryShortcuts ? "on" : "off";
        bpmPrecisionInput.value = Mixxx.Config.libraryBpmColumnPrecision;
        libraryRowHeightInput.value = Mixxx.Config.libraryRowHeight;
        trackPaletteComboBox.currentIndex = trackPaletteComboBox.model.indexOf(Mixxx.Config.configTrackColorPalette);
        hotcuePaletteComboBox.currentIndex = hotcuePaletteComboBox.model.indexOf(Mixxx.Config.configHotcueColorPalette);
        keyPaletteInput.selected = Mixxx.Config.configKeyColorsEnabled ? "on" : "off";
        keyPaletteComboBox.currentIndex = keyPaletteComboBox.model.indexOf(Mixxx.Config.configKeyColorPalette);
        colorPane.hotcuePaletteColorIndex = Mixxx.Config.controlHotcueDefaultColorIndex;
        if (colorPane.hotcuePaletteColorIndex < 0)
            colorPane.hotcuePaletteColorIndex = colorPane.defaultPalette.length - 1;
        hotcuePaletteInput.currentIndex = colorPane.hotcuePaletteColorIndex;
        colorPane.loopPaletteColorIndex = Mixxx.Config.controlLoopDefaultColorIndex;
        if (colorPane.loopPaletteColorIndex < 0)
            colorPane.loopPaletteColorIndex = colorPane.defaultPalette.length - 2;
        loopPaletteInput.currentIndex = colorPane.loopPaletteColorIndex;
        // Depends of #13216
        colorPane.jumpPaletteColorIndex = colorPane.defaultPalette.length - 3;
        jumpPaletteInput.currentIndex = colorPane.jumpPaletteColorIndex;
        themeColorTab.dirty = false;
    }
    function loadWaveform() {
        waveformTab.load({
            "endOfTrackWarning": Mixxx.Config.waveformEndOfTrackWarningTime,
            "beatGridAlpha": Mixxx.Config.waveformBeatGridAlpha,
            "defaultZoom": Mixxx.Config.waveformDefaultZoom,
            "zoomSynchronization": Mixxx.Config.waveformZoomSynchronization,
            "overviewNormalized": Mixxx.Config.waveformOverviewNormalized,
            "playMarkerPosition": Mixxx.Config.waveformPlayMarkerPosition,
            "untilMarkShowTime": Mixxx.Config.waveformUntilMarkShowTime,
            "untilMarkShowBeats": Mixxx.Config.waveformUntilMarkShowBeats,
            "untilMarkAlign": Mixxx.Config.waveformUntilMarkAlign,
            "untilMarkTextPointSize": Mixxx.Config.waveformUntilMarkTextPointSize,
            "visualGainAll": Mixxx.Config.waveformVisualGainAll,
            "visualGainLow": Mixxx.Config.waveformVisualGainLow,
            "visualGainMedium": Mixxx.Config.waveformVisualGainMedium,
            "visualGainHigh": Mixxx.Config.waveformVisualGainHigh,
            "type": Mixxx.Config.waveformType,
            "options": Mixxx.Config.waveformOptions
        });
    }
    function resetDeck() {
    }
    function resetInterface() {
    }
    function resetWaveform() {
        resetWaveformTab();
    }
    function resetWaveformTab() {
        waveformTab.load({
            "endOfTrackWarning": 30,
            "beatGridAlpha": 90,
            "defaultZoom": 3,
            "zoomSynchronization": true,
            "overviewNormalized": true,
            "playMarkerPosition": 0.5,
            "untilMarkShowTime": false,
            "untilMarkShowBeats": false,
            "untilMarkAlign": 1,
            "untilMarkTextPointSize": 24,
            "visualGainAll": 1,
            "visualGainLow": 1,
            "visualGainMedium": 1,
            "visualGainHigh": 1,
            "type": Mixxx.WaveformDisplay.RGB,
            "options": Mixxx.WaveformDisplay.Options.None
        });
        waveformTab.dirty = true;
    }
    function saveDeck() {
        Mixxx.Config.controlCueDefault = cueModeInput.currentIndex;
        Mixxx.Config.controlSetIntroStartAtMainCue = setIntroStartToMainCueInput.isOn;
        Mixxx.Config.controlPositionDisplay = trackTimeDisplayInput.options.indexOf(trackTimeDisplayInput.selected);
        Mixxx.Config.controlTimeFormat = timeMode.currentIndex;
        Mixxx.Config.controlCloneDeckOnLoadDoubleTap = doublePressLoadToCloneInput.selected === "on";
        Mixxx.Config.controlCueRecall = trackLoadPointInput.currentIndex;
        Mixxx.Config.controlLoadWhenDeckPlaying = loadingTrackWhenPlayingInput.options.indexOf(loadingTrackWhenPlayingInput.selected);
        Mixxx.Config.controlSpeedAutoReset = resetOnTrackLoadInput.options.indexOf(resetOnTrackLoadInput.selected);
        for (let i = 0; i < deckRateRange.count; i++) {
            deckRateRange.objectAt(i).value = sliderRangeInput.values[sliderRangeInput.currentIndex] / 100;
        }
        Mixxx.Config.controlRateRange = sliderRangeInput.values[sliderRangeInput.currentIndex];
        Mixxx.Config.bpmSyncLockAlgorithm = syncModeInput.options.indexOf(syncModeInput.selected);
        for (let i = 0; i < deckRateDirection.count; i++) {
            deckRateDirection.objectAt(i).value = sliderOrientationInput.selected === "down" ? -1 : 1;
        }
        Mixxx.Config.controlRateDir = sliderOrientationInput.selected === "down";
        Mixxx.Config.controlKeylockMode = keylockModeInput.options.indexOf(keylockModeInput.selected);
        Mixxx.Config.controlKeyunlockMode = keyunlockModeInput.options.indexOf(keyunlockModeInput.selected);
        Mixxx.Config.controlPitchBendBehaviour = pitchBendBehaviourInput.options.indexOf(pitchBendBehaviourInput.selected);
        Mixxx.Config.controlRateTempCoarse = adjustmentButtonsTemporaryCoarseInput.value / 100;
        Mixxx.Config.controlRateTempFine = adjustmentButtonsTemporaryFineInput.value / 100;
        Mixxx.Config.controlRatePermCoarse = adjustmentButtonsPermanentCoarseInput.value / 100;
        Mixxx.Config.controlRatePermFine = adjustmentButtonsPermanentFineInput.value / 100;
        Mixxx.Config.controlRateRampSensitivity = rampingSensitivityInput.value;
        loadDeck();
    }
    function saveInterface() {
        // Interface tab
        // skinInput.value =
        // colorInput.value =
        // layoutInput.value =
        Mixxx.Config.libraryTooltips = tooltipsInput.options.indexOf(tooltipsInput.selected);
        Mixxx.Config.libraryInhibitScreensaver = disableScreensaverInput.options.indexOf(disableScreensaverInput.selected);
        Mixxx.Config.configStartInFullscreenKey = startFullscreenInput.selected === "on";
        Mixxx.Config.libraryHideMenuBar = autoHideMenuBarInput.selected === "on";
        Mixxx.Config.libraryEnableSearchCompletions = searchCompletionInput.selected === "on";
        Mixxx.Config.libraryEnableSearchHistoryShortcuts = searchHistoryKeyboardInput.selected === "on";
        Mixxx.Config.libraryBpmColumnPrecision = bpmPrecisionInput.value;
        Mixxx.Config.libraryRowHeight = libraryRowHeightInput.value;
        Mixxx.Config.configTrackColorPalette = trackPaletteComboBox.model[trackPaletteComboBox.currentIndex];
        Mixxx.Config.configHotcueColorPalette = hotcuePaletteComboBox.model[hotcuePaletteComboBox.currentIndex];
        Mixxx.Config.configKeyColorsEnabled = keyPaletteInput.selected === "on";
        Mixxx.Config.configKeyColorPalette = keyPaletteComboBox.model[keyPaletteComboBox.currentIndex];
        // keyPaletteInput.value
        Mixxx.Config.controlHotcueDefaultColorIndex = hotcuePaletteInput.currentIndex;
        Mixxx.Config.controlLoopDefaultColorIndex = loopPaletteInput.currentIndex;
        // jumpPaletteInput.value =
        loadInterface();
    }
    function saveWaveform() {
        waveformTab.save();
    }

    label: "Interface"
    selectedIndex: 0
    tabs: ["theme & color", "waveform", "decks"]

    Component.onCompleted: {
        root.load();
    }
    onActivated: {
        waveformDropdown.track = waveformDropdown.player.isLoaded ? waveformDropdown.player.currentTrack : Mixxx.Library.model.getTrackByRow(Mixxx.Library.model.rowCount() * Math.random());
    }

    Mixxx.ControlProxy {
        id: numDecks

        group: "[App]"
        key: "num_decks"
    }
    Instantiator {
        id: deckRateRange

        model: numDecks.value

        Mixxx.ControlProxy {
            required property int index

            group: `[Channel${index + 1}]`
            key: "rateRange"
        }
    }
    Instantiator {
        id: deckRateDirection

        model: numDecks.value

        Mixxx.ControlProxy {
            required property int index

            group: `[Channel${index + 1}]`
            key: "rate_dir"
        }
    }
    component WaveformMaskComponent: Item {
        id: waveformMask

        property color rimColor: "#000000"
        property real size: 10

        anchors.fill: parent

        Rectangle {
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            height: waveformMask.size

            gradient: Gradient {
                orientation: Gradient.Vertical

                GradientStop {
                    color: waveformMask.rimColor
                    position: 0
                }
                GradientStop {
                    color: "transparent"
                    position: 1
                }
            }
        }
        Rectangle {
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            height: waveformMask.size

            gradient: Gradient {
                orientation: Gradient.Vertical

                GradientStop {
                    color: "transparent"
                    position: 0
                }
                GradientStop {
                    color: waveformMask.rimColor
                    position: 1
                }
            }
        }
        Rectangle {
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            width: waveformMask.size

            gradient: Gradient {
                orientation: Gradient.Horizontal

                GradientStop {
                    color: waveformMask.rimColor
                    position: 0
                }
                GradientStop {
                    color: "transparent"
                    position: 1
                }
            }
        }
        Rectangle {
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            width: waveformMask.size

            gradient: Gradient {
                orientation: Gradient.Horizontal

                GradientStop {
                    color: "transparent"
                    position: 0
                }
                GradientStop {
                    color: waveformMask.rimColor
                    position: 1
                }
            }
        }
    }
    ScrollView {
        id: scrollView

        ScrollBar.vertical: ScrollBar {
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            anchors.top: parent.top
            objectName: "interfaceSettingsScrollBar"
            policy: ScrollBar.AsNeeded
        }
        anchors.bottom: buttonActions.top
        anchors.bottomMargin: 18
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top

        Item {
            implicitHeight: [themeColorTab.implicitHeight, waveformTab.implicitHeight, decksTab.implicitHeight][root.selectedIndex]
            implicitWidth: scrollView.width

            Mixxx.SettingGroup {
                id: themeColorTab
                property bool dirty: false

                width: scrollView.width
                implicitHeight: themeColorTabColumn.y + themeColorTabColumn.height

                label: "Theme & Color"
                visible: root.selectedIndex == 0

                onActivated: {
                    root.selectedIndex = 0;
                }

                ColumnLayout {
                    y: 20
                    id: themeColorTabColumn
                    width: themeColorTab.width
                    spacing: 0

                    RowLayout {
                        id: theme

                        Layout.leftMargin: 14
                        Layout.rightMargin: 14
                        spacing: 20

                        ColumnLayout {
                            Layout.alignment: Qt.AlignTop
                            Layout.fillWidth: true
                            spacing: 15

                            RowLayout {
                                Mixxx.SettingParameter {
                                    Layout.fillWidth: true
                                    label: "Skin"

                                    Text {
                                        color: Theme.white
                                        font.pixelSize: 14
                                        text: parent.label
                                    }
                                }
                                Skin.ComboBox {
                                    id: skinInput

                                    model: ["Unnamed"]
                                    objectName: "setting_skin"

                                    onCurrentIndexChanged: themeColorTab.dirty = true
                                }
                            }
                            RowLayout {
                                Mixxx.SettingParameter {
                                    Layout.fillWidth: true
                                    label: "Color"

                                    Text {
                                        color: Theme.white
                                        font.pixelSize: 14
                                        text: parent.label
                                    }
                                }
                                Skin.ComboBox {
                                    id: colorInput

                                    model: ["Dark", "Light"]
                                    objectName: "setting_color"

                                    onCurrentIndexChanged: themeColorTab.dirty = true
                                }
                            }
                            RowLayout {
                                Mixxx.SettingParameter {
                                    Layout.fillWidth: true
                                    label: "Layout"

                                    Text {
                                        color: Theme.white
                                        font.pixelSize: 14
                                        text: parent.label
                                    }
                                }
                                Skin.ComboBox {
                                    id: layoutInput

                                    model: ["Performance", "Broadcast"]
                                    objectName: "setting_layout"

                                    onCurrentIndexChanged: themeColorTab.dirty = true
                                }
                            }
                            RowLayout {
                                Mixxx.SettingParameter {
                                    Layout.fillWidth: true
                                    label: "Tool tips"

                                    Text {
                                        color: Theme.white
                                        font.pixelSize: 14
                                        text: parent.label
                                    }
                                }
                                RatioChoice {
                                    id: tooltipsInput

                                    objectName: "setting_toolTips"
                                    options: ["off", "library", "all"]

                                    onSelectedChanged: themeColorTab.dirty = true
                                }
                            }
                            RowLayout {
                                Mixxx.SettingParameter {
                                    Layout.fillWidth: true
                                    label: "Disable screen saver"

                                    Text {
                                        color: Theme.white
                                        font.pixelSize: 14
                                        text: parent.label
                                    }
                                }
                                RatioChoice {
                                    id: disableScreensaverInput

                                    objectName: "setting_disableScreenSaver"
                                    options: ["no", "while running", "while playing"]

                                    onSelectedChanged: themeColorTab.dirty = true
                                }
                            }
                            RowLayout {
                                Mixxx.SettingParameter {
                                    Layout.fillWidth: true
                                    label: "Start in full-screen mode"

                                    Text {
                                        color: Theme.white
                                        font.pixelSize: 14
                                        text: parent.label
                                    }
                                }
                                RatioChoice {
                                    id: startFullscreenInput

                                    readonly property bool enabled: selected == "on"

                                    objectName: "setting_startFullscreen"
                                    options: ["on", "off"]

                                    onSelectedChanged: themeColorTab.dirty = true
                                }
                            }
                            RowLayout {
                                Layout.fillWidth: true

                                ColumnLayout {
                                    Layout.fillWidth: true

                                    Mixxx.SettingParameter {
                                        Layout.fillWidth: true
                                        height: 14
                                        label: "Auto-hide the menu bar"

                                        Text {
                                            color: Theme.white
                                            font.pixelSize: 14
                                            text: parent.label
                                        }
                                    }
                                    Text {
                                        color: Theme.white
                                        font.italic: true
                                        font.pixelSize: 12
                                        font.weight: Font.Thin
                                        text: "Toggle it with a single Alt key press"
                                    }
                                }
                                RatioChoice {
                                    id: autoHideMenuBarInput

                                    readonly property bool enabled: selected == "on"

                                    objectName: "setting_autoHideMenuBar"
                                    options: ["on", "off"]

                                    onSelectedChanged: themeColorTab.dirty = true
                                }
                            }
                        }
                        Item {
                            Layout.preferredWidth: root.width * 0.35

                            Rectangle {
                                anchors.horizontalCenter: parent.horizontalCenter
                                anchors.verticalCenter: parent.verticalCenter
                                color: '#343434'
                                height: width / 16 * 9
                                width: parent.width - 160

                                Skin.Button {
                                    activeColor: Theme.white
                                    anchors.bottom: parent.bottom
                                    anchors.bottomMargin: 20
                                    anchors.horizontalCenter: parent.horizontalCenter
                                    text: "Customize"
                                    width: 80
                                }
                            }
                        }
                    }
                    Mixxx.SettingGroup {
                        Layout.bottomMargin: 6
                        Layout.fillWidth: true
                        Layout.topMargin: 40
                        implicitHeight: libraryColumn.height
                        label: "Library"

                        Column {
                            id: libraryColumn

                            anchors.left: parent.left
                            anchors.right: parent.right

                            Text {
                                color: Theme.white
                                font.pixelSize: 14
                                font.weight: Font.DemiBold
                                text: "Library"
                            }
                            Item {
                                anchors.left: parent.left
                                anchors.right: parent.right
                                height: 10
                            }
                            Rectangle {
                                anchors.left: parent.left
                                anchors.right: parent.right
                                color: Theme.darkGray2
                                implicitHeight: libraryPane.implicitHeight + 20

                                GridLayout {
                                    id: libraryPane

                                    anchors.bottomMargin: 10
                                    anchors.fill: parent
                                    anchors.leftMargin: 17
                                    anchors.rightMargin: 17
                                    anchors.topMargin: 10
                                    columnSpacing: 80
                                    columns: 2
                                    rowSpacing: 15

                                    RowLayout {
                                        Layout.preferredWidth: libraryPane.width * 0.5

                                        Mixxx.SettingParameter {
                                            Layout.fillWidth: true
                                            implicitWidth: searchCompletionText.implicitWidth
                                            label: "Search completion"

                                            Text {
                                                id: searchCompletionText

                                                anchors.fill: parent
                                                color: Theme.white
                                                font.pixelSize: 14
                                                font.weight: Font.Medium
                                                horizontalAlignment: Text.AlignLeft
                                                text: parent.label
                                                verticalAlignment: Text.AlignVCenter
                                            }
                                        }
                                        RatioChoice {
                                            id: searchCompletionInput

                                            readonly property bool enabled: selected == "on"

                                            Layout.fillWidth: true
                                            objectName: "setting_searchCompletion"
                                            options: ["on", "off"]

                                            onSelectedChanged: themeColorTab.dirty = true
                                        }
                                    }
                                    RowLayout {
                                        Layout.preferredWidth: libraryPane.width * 0.5

                                        Mixxx.SettingParameter {
                                            Layout.fillWidth: true
                                            implicitWidth: searchHistoryText.implicitWidth
                                            label: "Search history keyboard shortcuts"

                                            Text {
                                                id: searchHistoryText

                                                anchors.fill: parent
                                                color: Theme.white
                                                font.pixelSize: 14
                                                font.weight: Font.Medium
                                                horizontalAlignment: Text.AlignLeft
                                                text: parent.label
                                                verticalAlignment: Text.AlignVCenter
                                            }
                                        }
                                        RatioChoice {
                                            id: searchHistoryKeyboardInput

                                            readonly property bool enabled: selected == "on"

                                            Layout.fillWidth: true
                                            objectName: "setting_searchHistoryKeyboardShortcuts"
                                            options: ["on", "off"]

                                            onSelectedChanged: themeColorTab.dirty = true
                                        }
                                    }
                                    RowLayout {
                                        Layout.preferredWidth: libraryPane.width * 0.5

                                        Mixxx.SettingParameter {
                                            Layout.fillWidth: true
                                            implicitWidth: bpmDisplayText.implicitWidth
                                            label: "BPM display precision"

                                            Text {
                                                id: bpmDisplayText

                                                anchors.fill: parent
                                                color: Theme.white
                                                font.pixelSize: 14
                                                font.weight: Font.Medium
                                                horizontalAlignment: Text.AlignLeft
                                                text: parent.label
                                                verticalAlignment: Text.AlignVCenter
                                            }
                                        }
                                        SettingComponents.SpinBox {
                                            id: bpmPrecisionInput

                                            Layout.fillWidth: true
                                            editable: false
                                            implicitWidth: 180
                                            max: 10
                                            min: 0
                                            objectName: "setting_bpmDisplayPrecision"
                                            precision: 0
                                            realValue: 1

                                            contentItem: Item {
                                                Rectangle {
                                                    id: content

                                                    anchors.fill: parent
                                                    color: Theme.accentColor
                                                    border {
                                                        color: "#0E2A54"
                                                        width: 1
                                                    }

                                                    Text {
                                                        id: textLabel

                                                        anchors.fill: parent
                                                        color: Theme.white
                                                        font: bpmPrecisionInput.font
                                                        horizontalAlignment: Text.AlignHCenter
                                                        text: (126.0).toFixed(bpmPrecisionInput.value)
                                                        verticalAlignment: Text.AlignVCenter
                                                    }
                                                }
                                            }

                                            onValueChanged: themeColorTab.dirty = true
                                        }
                                    }
                                    RowLayout {
                                        Layout.preferredWidth: libraryPane.width * 0.5

                                        Mixxx.SettingParameter {
                                            Layout.fillWidth: true
                                            label: "Library Row Height"

                                            Text {
                                                anchors.fill: parent
                                                color: Theme.white
                                                font.pixelSize: 14
                                                font.weight: Font.Medium
                                                horizontalAlignment: Text.AlignLeft
                                                text: parent.label
                                                verticalAlignment: Text.AlignVCenter
                                            }
                                        }
                                        SettingComponents.Slider {
                                            id: libraryRowHeightInput

                                            markers: [14, 20, 50, 80]
                                            max: 100
                                            min: 14
                                            objectName: "setting_libraryRowHeight"
                                            suffix: "px"
                                            value: 14
                                            width: 300

                                            onValueChanged: themeColorTab.dirty = true
                                        }
                                    }
                                }
                            }
                        }
                    }
                    Mixxx.SettingGroup {
                        Layout.bottomMargin: 6
                        Layout.fillWidth: true
                        Layout.topMargin: 40
                        implicitHeight: colorColumn.height
                        label: "Colors"

                        Column {
                            id: colorColumn

                            anchors.left: parent.left
                            anchors.right: parent.right

                            Text {
                                color: Theme.white
                                font.pixelSize: 14
                                font.weight: Font.DemiBold
                                text: "Colors"
                            }
                            Item {
                                anchors.left: parent.left
                                anchors.right: parent.right
                                height: 10
                            }
                            Rectangle {
                                anchors.left: parent.left
                                anchors.right: parent.right
                                color: Theme.darkGray2
                                implicitHeight: colorPane.implicitHeight + 20

                                GridLayout {
                                    id: colorPane

                                    readonly property var defaultPalette: Mixxx.Config.getHotcueColorPalette(hotcuePaletteComboBox.currentText)
                                    property int hotcuePaletteColorIndex: 0
                                    property int jumpPaletteColorIndex: 0
                                    property int loopPaletteColorIndex: 0

                                    anchors.bottomMargin: 10
                                    anchors.fill: parent
                                    anchors.leftMargin: 3
                                    anchors.rightMargin: 3
                                    anchors.topMargin: 10
                                    columnSpacing: 20
                                    columns: 2
                                    rowSpacing: 8

                                    onDefaultPaletteChanged: {
                                        hotcuePaletteColorIndex = 0;
                                        loopPaletteColorIndex = 1;
                                        jumpPaletteColorIndex = 2;
                                    }

                                    RowLayout {
                                        Layout.leftMargin: 14
                                        Layout.preferredWidth: (colorPane.width - 56) * 0.5
                                        Layout.rightMargin: 14

                                        Mixxx.SettingParameter {
                                            Layout.fillWidth: true
                                            label: "Track palette"

                                            Text {
                                                anchors.fill: parent
                                                color: Theme.white
                                                font.pixelSize: 14
                                                font.weight: Font.Medium
                                                horizontalAlignment: Text.AlignLeft
                                                text: parent.label
                                                verticalAlignment: Text.AlignVCenter
                                            }
                                        }
                                        ColorPaletteComboBox {
                                            id: trackPaletteComboBox

                                            objectName: "setting_trackPalette"

                                            onCurrentIndexChanged: themeColorTab.dirty = true
                                        }
                                    }
                                    Rectangle {
                                        Layout.preferredWidth: (colorPane.width - 56) * 0.5
                                        Layout.rightMargin: 14
                                        color: Theme.darkGray3
                                        implicitHeight: 45

                                        ColumnLayout {
                                            anchors.fill: parent
                                            spacing: 0

                                            Mixxx.SettingParameter {
                                                Layout.fillWidth: true
                                                implicitHeight: hotcueDefaultColorText.implicitHeight
                                                implicitWidth: hotcueDefaultColorText.implicitWidth
                                                label: "Hotcue default color"

                                                Text {
                                                    id: hotcueDefaultColorText

                                                    anchors.fill: parent
                                                    color: Theme.white
                                                    font.pixelSize: 14
                                                    font.weight: Font.Medium
                                                    horizontalAlignment: Text.AlignHCenter
                                                    text: parent.label
                                                    verticalAlignment: Text.AlignVCenter
                                                }
                                            }
                                            DefaultColorSelector {
                                                id: hotcuePaletteInput

                                                Layout.alignment: Qt.AlignHCenter
                                                Layout.preferredWidth: colorPane.defaultPalette.length * 20
                                                currentIndex: colorPane.hotcuePaletteColorIndex
                                                objectName: "setting_hotcueDefaultColor"
                                                palette: colorPane.defaultPalette

                                                onCurrentIndexChanged: themeColorTab.dirty = true
                                            }
                                        }
                                    }
                                    RowLayout {
                                        Layout.leftMargin: 14
                                        Layout.preferredWidth: (colorPane.width - 56) * 0.5
                                        Layout.rightMargin: 14

                                        Mixxx.SettingParameter {
                                            Layout.fillWidth: true
                                            implicitWidth: keyColorText.implicitWidth
                                            label: "Key color"

                                            Text {
                                                id: keyColorText

                                                anchors.fill: parent
                                                color: Theme.white
                                                font.pixelSize: 14
                                                font.weight: Font.Medium
                                                horizontalAlignment: Text.AlignLeft
                                                text: parent.label
                                                verticalAlignment: Text.AlignVCenter
                                            }
                                        }
                                        RatioChoice {
                                            id: keyPaletteInput

                                            readonly property bool enabled: selected == "on"

                                            objectName: "setting_keyColor"
                                            options: ["on", "off"]

                                            onSelectedChanged: themeColorTab.dirty = true
                                        }
                                        ColorPaletteComboBox {
                                            id: keyPaletteComboBox

                                            enabled: keyPaletteInput.enabled
                                            // model: Mixxx.Config.paletteNames.filter(palette => Mixxx.Config.colorPalette(palette).length == 12)
                                            objectName: "setting_keyColorPalette"
                                            opacity: enabled ? 1 : 0.4

                                            onCurrentIndexChanged: themeColorTab.dirty = true
                                        }
                                    }
                                    Rectangle {
                                        Layout.preferredWidth: (colorPane.width - 56) * 0.5
                                        Layout.rightMargin: 14
                                        color: Theme.darkGray3
                                        implicitHeight: 45

                                        ColumnLayout {
                                            anchors.fill: parent
                                            spacing: 0

                                            Mixxx.SettingParameter {
                                                Layout.fillWidth: true
                                                implicitHeight: loopDefaultColorText.implicitHeight
                                                implicitWidth: loopDefaultColorText.implicitWidth
                                                label: "Loop default color"

                                                Text {
                                                    id: loopDefaultColorText

                                                    anchors.fill: parent
                                                    color: Theme.white
                                                    font.pixelSize: 14
                                                    font.weight: Font.Medium
                                                    horizontalAlignment: Text.AlignHCenter
                                                    text: parent.label
                                                    verticalAlignment: Text.AlignVCenter
                                                }
                                            }
                                            DefaultColorSelector {
                                                id: loopPaletteInput

                                                Layout.alignment: Qt.AlignHCenter
                                                Layout.preferredWidth: colorPane.defaultPalette.length * 20
                                                currentIndex: colorPane.loopPaletteColorIndex
                                                objectName: "setting_loopDefaultColor"
                                                palette: colorPane.defaultPalette

                                                onCurrentIndexChanged: themeColorTab.dirty = true
                                            }
                                        }
                                    }
                                    RowLayout {
                                        Layout.leftMargin: 14
                                        Layout.preferredWidth: (colorPane.width - 56) * 0.5
                                        Layout.rightMargin: 14

                                        Mixxx.SettingParameter {
                                            Layout.fillWidth: true
                                            label: "Hotcue palette"

                                            Text {
                                                anchors.fill: parent
                                                color: Theme.white
                                                font.pixelSize: 14
                                                font.weight: Font.Medium
                                                horizontalAlignment: Text.AlignLeft
                                                text: parent.label
                                                verticalAlignment: Text.AlignVCenter
                                            }
                                        }
                                        ColorPaletteComboBox {
                                            id: hotcuePaletteComboBox

                                            objectName: "setting_hotcuePalette"

                                            onCurrentIndexChanged: themeColorTab.dirty = true
                                        }
                                    }
                                    Rectangle {
                                        Layout.preferredWidth: (colorPane.width - 56) * 0.5
                                        Layout.rightMargin: 14
                                        color: Theme.darkGray3
                                        implicitHeight: 45

                                        ColumnLayout {
                                            anchors.fill: parent
                                            spacing: 0

                                            Mixxx.SettingParameter {
                                                Layout.fillWidth: true
                                                implicitHeight: jumpDefaultColorText.implicitHeight
                                                implicitWidth: jumpDefaultColorText.implicitWidth
                                                label: "Jump default color"

                                                Text {
                                                    id: jumpDefaultColorText

                                                    anchors.fill: parent
                                                    color: Theme.white
                                                    font.pixelSize: 14
                                                    font.weight: Font.Medium
                                                    horizontalAlignment: Text.AlignHCenter
                                                    text: parent.label
                                                    verticalAlignment: Text.AlignVCenter
                                                }
                                            }
                                            DefaultColorSelector {
                                                id: jumpPaletteInput

                                                Layout.alignment: Qt.AlignHCenter
                                                Layout.preferredWidth: colorPane.defaultPalette.length * 20
                                                currentIndex: colorPane.jumpPaletteColorIndex
                                                objectName: "setting_jumpDefaultColor"
                                                palette: colorPane.defaultPalette

                                                onCurrentIndexChanged: themeColorTab.dirty = true
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                    Item {
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                    }
                }
            }
            Mixxx.SettingGroup {
                id: waveformTab

                objectName: "waveformSettingsTab"

                readonly property real beatgridOpacity: beatGridAlphaInput.value / 100
                readonly property real defaultZoom: defaultZoomInput.value / 10
                property bool dirty: false
                readonly property color colorCode: waveformDropdown.colorCode.toString()
                readonly property color highColorCode: waveformDropdown.highColor.toString()
                readonly property color lowColorCode: waveformDropdown.lowColor.toString()
                readonly property color midColorCode: waveformDropdown.midColor.toString()
                readonly property double globalGain: visualGainAllInput.value / 100
                readonly property double highGain: visualGainHighInput.value / 100
                readonly property double lowGain: visualGainLowInput.value / 100
                readonly property double middleGain: visualGainMediumInput.value / 100
                readonly property int playMarkerAlign: untilMarkAlignInput.selected === "top" ? Qt.AlignTop : untilMarkAlignInput.selected === "bottom" ? Qt.AlignBottom : Qt.AlignVCenter
                readonly property real playMarkerPosition: playMarkerPositionInput.value / 100
                readonly property double playMarkerTextSize: untilMarkTextPointSizeInput.value
                readonly property bool untilMarkShowBeats: untilMarkShowBeatsInput.isOn
                readonly property bool untilMarkShowTime: untilMarkShowTimeInput.isOn

                function load(value) {
                    endOfTrackWarningInput.value = value.endOfTrackWarning;
                    beatGridAlphaInput.value = value.beatGridAlpha;
                    defaultZoomInput.value = value.defaultZoom * 10;
                    synchronizeAllZoomLevelInput.selected = value.zoomSynchronization ? "on" : "off";
                    normaliseWaveformOverviewLevelInput.selected = value.overviewNormalized ? "on" : "off";
                    playMarkerPositionInput.value = value.playMarkerPosition * 100;
                    untilMarkShowTimeInput.load(value.untilMarkShowTime);
                    untilMarkShowBeatsInput.load(value.untilMarkShowBeats);
                    untilMarkAlignInput.load(value.untilMarkAlign);
                    let textPointSize = value.untilMarkTextPointSize;
                    if (!untilMarkTextPointSizeInput.fontSizes.includes(textPointSize)) {
                        untilMarkTextPointSizeInput.fontSizes.push(textPointSize);
                        untilMarkTextPointSizeInput.fontSizes.sort((a, b) => a - b);
                    }
                    untilMarkTextPointSizeInput.currentIndex = untilMarkTextPointSizeInput.fontSizes.indexOf(textPointSize);
                    visualGainAllInput.value = value.visualGainAll * 100;
                    visualGainLowInput.value = value.visualGainLow * 100;
                    visualGainMediumInput.value = value.visualGainMedium * 100;
                    visualGainHighInput.value = value.visualGainHigh * 100;
                    waveformDropdown.currentIndex = waveformDropdown.indices.indexOf(value.type);
                    waveformOptionHighDetailsInput.selected = (value.options & Mixxx.WaveformDisplay.Options.HighDetail) ? "on" : "off";
                    waveformOptionStereoInput.selected = (value.options & Mixxx.WaveformDisplay.Options.SplitStereoSignal) ? "on" : "off";
                    waveformTab.dirty = false;
                }
                function save() {
                    Mixxx.Config.waveformEndOfTrackWarningTime = endOfTrackWarningInput.value;
                    Mixxx.Config.waveformZoomSynchronization = synchronizeAllZoomLevelInput.isOn;
                    Mixxx.Config.waveformOverviewNormalized = normaliseWaveformOverviewLevelInput.isOn;
                    Mixxx.Config.waveformPlayMarkerPosition = waveformTab.playMarkerPosition;
                    Mixxx.Config.waveformDefaultZoom = waveformTab.defaultZoom;
                    Mixxx.Config.waveformBeatGridAlpha = beatGridAlphaInput.value;
                    Mixxx.Config.waveformUntilMarkShowTime = waveformTab.untilMarkShowTime;
                    Mixxx.Config.waveformUntilMarkShowBeats = waveformTab.untilMarkShowBeats;
                    Mixxx.Config.waveformUntilMarkAlign = untilMarkAlignInput.options.indexOf(untilMarkAlignInput.selected);
                    Mixxx.Config.waveformUntilMarkTextPointSize = waveformTab.playMarkerTextSize;

                    Mixxx.Config.waveformVisualGainAll = visualGainAllInput.value / 100;
                    Mixxx.Config.waveformVisualGainLow = visualGainLowInput.value / 100;
                    Mixxx.Config.waveformVisualGainMedium = visualGainMediumInput.value / 100;
                    Mixxx.Config.waveformVisualGainHigh = visualGainHighInput.value / 100;

                    Mixxx.Config.waveformType = waveformDropdown.indices[waveformDropdown.currentIndex];
                    Mixxx.Config.waveformOptions = (waveformOptionHighDetailsInput.isOn ? Mixxx.WaveformDisplay.Options.HighDetail : 0) | (waveformOptionStereoInput.isOn ? Mixxx.WaveformDisplay.Options.SplitStereoSignal : 0);
                    waveformTab.load(waveformTabCfg());
                }
                function waveformTabCfg() {
                    return {
                        "endOfTrackWarning": endOfTrackWarningInput.value,
                        "beatGridAlpha": beatGridAlphaInput.value,
                        "defaultZoom": defaultZoomInput.value / 10,
                        "zoomSynchronization": synchronizeAllZoomLevelInput.isOn,
                        "overviewNormalized": normaliseWaveformOverviewLevelInput.isOn,
                        "playMarkerPosition": playMarkerPositionInput.value / 100,
                        "untilMarkShowTime": untilMarkShowTimeInput.selected === "on",
                        "untilMarkShowBeats": untilMarkShowBeatsInput.selected === "on",
                        "untilMarkAlign": untilMarkAlignInput.options.indexOf(untilMarkAlignInput.selected),
                        "untilMarkTextPointSize": untilMarkTextPointSizeInput.value,
                        "visualGainAll": visualGainAllInput.value / 100,
                        "visualGainLow": visualGainLowInput.value / 100,
                        "visualGainMedium": visualGainMediumInput.value / 100,
                        "visualGainHigh": visualGainHighInput.value / 100,
                        "type": waveformDropdown.indices[waveformDropdown.currentIndex],
                        "options": (waveformOptionHighDetailsInput.isOn ? Mixxx.WaveformDisplay.Options.HighDetail : 0) | (waveformOptionStereoInput.isOn ? Mixxx.WaveformDisplay.Options.SplitStereoSignal : 0)
                    };
                }

                anchors.fill: parent
                label: "Waveform"
                visible: root.selectedIndex == 1

                width: scrollView.width
                implicitHeight: waveformTabColumn.y + waveformTabColumn.height

                onActivated: {
                    root.selectedIndex = 1;
                }

                ColumnLayout {
                    id: waveformTabColumn
                    anchors.fill: parent
                    anchors.topMargin: 20

                    width: themeColorTab.width

                    RowLayout {
                        Layout.leftMargin: 14
                        Layout.rightMargin: 14

                        ColumnLayout {
                            Layout.preferredWidth: root.width * 0.5
                            spacing: 15

                            GridLayout {
                                columns: waveformTab.width < 1000 ? 1 : 2
                                rowSpacing: 10

                                Mixxx.SettingParameter {
                                    Layout.fillWidth: true
                                    height: 14
                                    label: "End of track warning"

                                    Text {
                                        color: Theme.white
                                        font.pixelSize: 14
                                        text: parent.label
                                    }
                                }
                                SettingComponents.Slider {
                                    id: endOfTrackWarningInput

                                    Layout.preferredWidth: parent.columns === 1 ? undefined : waveformTab.width * 0.35
                                    Layout.fillWidth: parent.columns === 1

                                    markers: [0, 10, 30, 60, 120]
                                    max: 120
                                    objectName: "setting_endOfTrackWarning"
                                    suffix: "sec"
                                    value: 30

                                    onValueChanged: waveformTab.dirty = true
                                }
                                Mixxx.SettingParameter {
                                    Layout.fillWidth: true
                                    height: 14
                                    label: "Beat grid opacity"

                                    Text {
                                        color: Theme.white
                                        font.pixelSize: 14
                                        text: parent.label
                                    }
                                }
                                SettingComponents.Slider {
                                    id: beatGridAlphaInput

                                    Layout.preferredWidth: parent.columns === 1 ? undefined : waveformTab.width * 0.35
                                    Layout.fillWidth: parent.columns === 1

                                    markers: [0, 50, 100]
                                    objectName: "setting_beatGridAlpha"
                                    suffix: "%"
                                    value: 50

                                    onValueChanged: waveformTab.dirty = true
                                }
                                Mixxx.SettingParameter {
                                    Layout.fillWidth: true
                                    height: 14
                                    label: "Default zoom level"

                                    Text {
                                        color: Theme.white
                                        font.pixelSize: 14
                                        text: parent.label
                                    }
                                }
                                SettingComponents.Slider {
                                    id: defaultZoomInput

                                    Layout.preferredWidth: parent.columns === 1 ? undefined : waveformTab.width * 0.35
                                    Layout.fillWidth: parent.columns === 1

                                    markers: [10, 30, 50, 100]
                                    objectName: "setting_defaultZoom"
                                    suffix: "%"
                                    value: 30

                                    onValueChanged: waveformTab.dirty = true
                                }
                            }
                            ColumnLayout {
                                RowLayout {
                                    Mixxx.SettingParameter {
                                        Layout.fillWidth: true
                                        height: 14
                                        label: "Synchronise zoom level across waveform"

                                        Text {
                                            color: Theme.white
                                            font.pixelSize: 14
                                            text: parent.label
                                        }
                                    }
                                    RatioChoice {
                                        id: synchronizeAllZoomLevelInput

                                        readonly property bool isOn: selected == "on"

                                        objectName: "setting_zoomSynchronization"
                                        options: ["on", "off"]

                                        onSelectedChanged: waveformTab.dirty = true
                                    }
                                }
                                RowLayout {
                                    Mixxx.SettingParameter {
                                        Layout.fillWidth: true
                                        height: 14
                                        label: "Normalise waveform overview"

                                        Text {
                                            color: Theme.white
                                            font.pixelSize: 14
                                            text: parent.label
                                        }
                                    }
                                    RatioChoice {
                                        id: normaliseWaveformOverviewLevelInput

                                        readonly property bool isOn: selected == "on"

                                        objectName: "setting_overviewNormalized"
                                        options: ["on", "off"]

                                        onSelectedChanged: waveformTab.dirty = true
                                    }
                                }
                            }
                        }
                        ColumnLayout {
                            Layout.preferredWidth: root.width * 0.5

                            ComboBox {
                                id: waveformDropdown

                                objectName: "setting_waveformType"

                                onWidthChanged: {
                                    print(waveformDropdown.width)
                                }

                                property var axesColor: '#a1a1a1a1'
                                property var backgroundColor: "#0F0F0E"
                                property var beatsRenderer: Mixxx.WaveformRendererBeat {
                                    color: Qt.alpha('#a1a1a1', waveformTab.beatgridOpacity)
                                }
                                readonly property var currentSignalRender: (currentIndex == 0 ? filteredRenderer : currentIndex == 1 ? hsvRenderer : currentIndex == 2 ? rgbRenderer : currentIndex == 3 ? simpleRenderer : stackedRenderer)
                                readonly property var currentType: indices[currentIndex]
                                property var filteredRenderer: Mixxx.WaveformRendererFiltered {
                                    axesColor: waveformDropdown.axesColor
                                    gainAll: waveformTab.globalGain
                                    gainHigh: waveformTab.highGain
                                    gainLow: waveformTab.lowGain
                                    gainMid: waveformTab.middleGain
                                    highColor: waveformDropdown.highColor
                                    ignoreStem: true
                                    lowColor: waveformDropdown.lowColor
                                    midColor: waveformDropdown.midColor
                                }
                                property var highColor: '#D5C2A2'
                                property var hsvRenderer: Mixxx.WaveformRendererHSV {
                                    axesColor: waveformDropdown.axesColor
                                    color: waveformDropdown.lowColor
                                    gainAll: waveformTab.globalGain
                                    gainHigh: waveformTab.highGain
                                    gainLow: waveformTab.lowGain
                                    gainMid: waveformTab.middleGain
                                    ignoreStem: true
                                }
                                readonly property list<var> indices: [Mixxx.WaveformDisplay.Filtered, Mixxx.WaveformDisplay.HSV, Mixxx.WaveformDisplay.RGB, Mixxx.WaveformDisplay.Simple, Mixxx.WaveformDisplay.Stacked,]
                                property var lowColor: '#2154D7'
                                property var midColor: '#97632D'
                                property int options: Mixxx.WaveformDisplay.Options.None
                                property double playbackProgress: 0.2
                                readonly property var player: Mixxx.PlayerManager.getPlayer("[Channel1]")
                                property var rgbRenderer: Mixxx.WaveformRendererRGB {
                                    axesColor: waveformDropdown.axesColor
                                    gainAll: waveformTab.globalGain
                                    gainHigh: waveformTab.highGain
                                    gainLow: waveformTab.lowGain
                                    gainMid: waveformTab.middleGain
                                    highColor: waveformDropdown.highColor
                                    ignoreStem: true
                                    lowColor: waveformDropdown.lowColor
                                    midColor: waveformDropdown.midColor
                                }
                                property var simpleRenderer: Mixxx.WaveformRendererSimple {
                                    axesColor: waveformDropdown.axesColor
                                    color: waveformDropdown.lowColor
                                    gain: waveformTab.globalGain
                                    ignoreStem: true
                                }
                                property var stackedRenderer: Mixxx.WaveformRendererFiltered {
                                    axesColor: waveformDropdown.axesColor
                                    gainAll: waveformTab.globalGain
                                    gainHigh: waveformTab.highGain
                                    gainLow: waveformTab.lowGain
                                    gainMid: waveformTab.middleGain
                                    highColor: waveformDropdown.highColor
                                    ignoreStem: true
                                    lowColor: waveformDropdown.lowColor
                                    midColor: waveformDropdown.midColor
                                    stacked: true
                                }
                                property var track: null

                                Layout.fillWidth: true
                                Layout.preferredHeight: 102
                                model: ["Filtered", "HSV", "RGB", "Simple", "Stacked"]

                                background: Rectangle {
                                    // border.width: control.highlighted ?  1 : 0
                                    // border.color: Theme.deckLineColor
                                    color: "#000000"
                                    radius: 5
                                }
                                contentItem: Rectangle {
                                    color: '#00000000'

                                    ColumnLayout {
                                        anchors.fill: parent

                                        Item {
                                            property var markRenderer: Mixxx.WaveformRendererMark {
                                                playMarkerBackground: '#D9D9D9'
                                                playMarkerColor: '#D9D9D9'
                                                playMarkerPosition: waveformTab.playMarkerPosition
                                                untilMark.align: waveformTab.playMarkerAlign
                                                untilMark.defaultNextMarkPosition: waveformDropdown.player.isLoaded || waveformDropdown.track == null ? -1 : waveformDropdown.track.duration * waveformDropdown.track.sampleRate * 2
                                                untilMark.showBeats: waveformTab.untilMarkShowBeats
                                                untilMark.showTime: waveformTab.untilMarkShowTime
                                                untilMark.textSize: waveformTab.playMarkerTextSize

                                                defaultMark: Mixxx.WaveformMark {
                                                    align: "bottom|right"
                                                    color: "#00d9ff"
                                                    text: " %1 "
                                                    textColor: "#1a1a1a"
                                                }
                                            }

                                            Layout.fillHeight: true
                                            Layout.fillWidth: true

                                            Mixxx.WaveformDisplay {
                                                id: previewWaveform
                                                objectName: "waveformPreview"

                                                anchors.fill: parent
                                                backgroundColor: "#0F0F0E"
                                                options: (waveformOptionHighDetailsInput.isOn ? Mixxx.WaveformDisplay.Options.HighDetail : 0) + (waveformOptionStereoInput.isOn ? Mixxx.WaveformDisplay.Options.SplitStereoSignal : 0)
                                                player: waveformDropdown.player.isLoaded ? waveformDropdown.player : null
                                                position: waveformDropdown.playbackProgress
                                                renderers: [waveformDropdown.currentSignalRender, waveformDropdown.beatsRenderer, parent.markRenderer]
                                                track: waveformDropdown.player.isLoaded ? null : waveformDropdown.track
                                                zoom: waveformTab.defaultZoom

                                                // Test feedback surface: what the
                                                // embedded preview actually
                                                // renders, read back through the
                                                // renderers bound into it.
                                                readonly property real beatGridOpacity: waveformDropdown.beatsRenderer.color.a
                                                readonly property real playMarkerPosition: parent.markRenderer.playMarkerPosition
                                                readonly property string untilMarkAlignName: parent.markRenderer.untilMark.align === Qt.AlignTop ? "top" : parent.markRenderer.untilMark.align === Qt.AlignBottom ? "bottom" : "center"
                                                readonly property bool untilMarkShowBeats: parent.markRenderer.untilMark.showBeats
                                                readonly property bool untilMarkShowTime: parent.markRenderer.untilMark.showTime
                                                readonly property real untilMarkTextSize: parent.markRenderer.untilMark.textSize
                                                // The Simple renderer folds every
                                                // band into a single gain and has
                                                // no per-band colors.
                                                readonly property real gainAll: waveformDropdown.currentSignalRender.gainAll !== undefined ? waveformDropdown.currentSignalRender.gainAll : waveformDropdown.currentSignalRender.gain
                                                readonly property real gainHigh: waveformDropdown.currentSignalRender.gainHigh !== undefined ? waveformDropdown.currentSignalRender.gainHigh : waveformDropdown.currentSignalRender.gain
                                                readonly property real gainLow: waveformDropdown.currentSignalRender.gainLow !== undefined ? waveformDropdown.currentSignalRender.gainLow : waveformDropdown.currentSignalRender.gain
                                                readonly property real gainMid: waveformDropdown.currentSignalRender.gainMid !== undefined ? waveformDropdown.currentSignalRender.gainMid : waveformDropdown.currentSignalRender.gain
                                                readonly property color colorCode: waveformDropdown.currentSignalRender.color
                                                readonly property color lowColorCode: waveformDropdown.currentSignalRender.lowColor
                                                readonly property color midColorCode: waveformDropdown.currentSignalRender.midColor
                                                readonly property color highColorCode: waveformDropdown.currentSignalRender.highColor
                                            }
                                            WaveformMaskComponent {
                                                anchors.fill: parent
                                            }
                                        }
                                        RowLayout {
                                            Layout.fillWidth: true

                                            Text {
                                                Layout.fillWidth: true
                                                Layout.margins: 5
                                                color: "#FFFFFF"
                                                font.weight: Font.DemiBold
                                                text: waveformDropdown.currentText
                                            }
                                            GridLayout {
                                                columnSpacing: 0
                                                rowSpacing: 0
                                                columns: waveformDropdown.width > 500 ? 2 : 1
                                                Text {
                                                    Layout.alignment: Qt.AlignHCenter
                                                    color: "#FFFFFF"
                                                    font.pixelSize: 11
                                                    text: "Stereo split"
                                                    visible: waveformDropdown.currentSignalRender.supportedOptions & Mixxx.WaveformDisplay.Options.SplitStereoSignal
                                                }
                                                RatioChoice {
                                                    id: waveformOptionStereoInput

                                                    readonly property bool isOn: selected == "on"

                                                    content.height: 18
                                                    inactiveColor: Theme.darkGray4
                                                    metric.font.pixelSize: 11
                                                    objectName: "setting_stereoSplit"
                                                    options: ["on", "off"]
                                                    visible: waveformDropdown.currentSignalRender.supportedOptions & Mixxx.WaveformDisplay.Options.SplitStereoSignal

                                                    onSelectedChanged: waveformTab.dirty = true
                                                }
                                            }
                                            GridLayout {
                                                columnSpacing: 0
                                                rowSpacing: 0
                                                columns: waveformDropdown.width > 500 ? 2 : 1
                                                Text {
                                                    Layout.alignment: Qt.AlignHCenter
                                                    color: "#FFFFFF"
                                                    font.pixelSize: 11
                                                    text: "High details"
                                                    visible: waveformDropdown.currentSignalRender.supportedOptions & Mixxx.WaveformDisplay.Options.HighDetail
                                                }
                                                RatioChoice {
                                                    id: waveformOptionHighDetailsInput

                                                    readonly property bool isOn: selected == "on"

                                                    content.height: 18
                                                    inactiveColor: Theme.darkGray4
                                                    metric.font.pixelSize: 11
                                                    objectName: "setting_highDetails"
                                                    options: ["on", "off"]
                                                    visible: waveformDropdown.currentSignalRender.supportedOptions & Mixxx.WaveformDisplay.Options.HighDetail

                                                    onSelectedChanged: waveformTab.dirty = true
                                                }
                                            }
                                            GridLayout {
                                                columnSpacing: 0
                                                rowSpacing: 0
                                                columns: waveformDropdown.width > 500 ? 2 : 1
                                                Text {
                                                    Layout.alignment: Qt.AlignHCenter
                                                    color: "#FFFFFF"
                                                    font.pixelSize: 11
                                                    text: "Color"
                                                }
                                                RowLayout {
                                                    Layout.margins: 4
                                                    implicitHeight: 18
                                                    Repeater {
                                                        model: waveformDropdown.currentSignalRender.color === undefined ? [waveformDropdown.lowColor, waveformDropdown.midColor, waveformDropdown.highColor] : [waveformDropdown.lowColor]

                                                        Rectangle {
                                                            required property int index
                                                            required property color modelData

                                                            // Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
                                                            Layout.margins: 1
                                                            color: modelData
                                                            height: 15
                                                            objectName: "waveformColorSwatch_" + this.index
                                                            radius: 2
                                                            width: 15

                                                            ColorDialog {
                                                                id: colorDialog

                                                                options: ColorDialog.ShowAlphaChannel
                                                                selectedColor: modelData
                                                                title: "Please choose a color"

                                                                onAccepted: {
                                                                    if (index == 0) {
                                                                        waveformDropdown.lowColor = Qt.color(selectedColor);
                                                                    } else if (index == 1) {
                                                                        waveformDropdown.midColor = Qt.color(selectedColor);
                                                                    } else {
                                                                        waveformDropdown.highColor = Qt.color(selectedColor);
                                                                    }
                                                                }
                                                            }
                                                            MouseArea {
                                                                anchors.fill: parent
                                                                cursorShape: Qt.PointingHandCursor

                                                                onPressed: {
                                                                    if (colorPickerTest.testMode) {
                                                                        colorPickerTest.open(index);
                                                                    } else {
                                                                        colorDialog.open();
                                                                    }
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                    // leftPadding: 5
                                    // rightPadding: waveformDropdown.indicator.width + waveformDropdown.spacing
                                }
                                delegate: ItemDelegate {
                                    id: control

                                    required property int index

                                    height: 99
                                    highlighted: waveformDropdown.highlightedIndex === this.index
                                    objectName: "setting_waveformType_option_" + this.index
                                    width: parent.width

                                    background: Rectangle {
                                        color: control.pressed || control.highlighted ? "#575757" : "#000000"
                                        radius: 5
                                    }

                                    ColumnLayout {
                                        anchors.fill: parent

                                        Item {
                                            Layout.fillHeight: true
                                            Layout.fillWidth: true

                                            Mixxx.WaveformDisplay {
                                                id: waveform

                                                anchors.fill: parent
                                                backgroundColor: "#0F0F0E"
                                                options: (waveformOptionHighDetailsInput.isOn ? Mixxx.WaveformDisplay.Options.HighDetail : 0) | (waveformOptionStereoInput.isOn ? Mixxx.WaveformDisplay.Options.SplitStereoSignal : 0)
                                                player: waveformDropdown.player.isLoaded ? waveformDropdown.player : null
                                                position: waveformDropdown.playbackProgress
                                                renderers: [(index == 0 ? waveformDropdown.filteredRenderer : index == 1 ? waveformDropdown.hsvRenderer : index == 2 ? waveformDropdown.rgbRenderer : index == 3 ? waveformDropdown.simpleRenderer : waveformDropdown.stackedRenderer), waveformDropdown.beatsRenderer]
                                                track: waveformDropdown.player.isLoaded ? null : waveformDropdown.track
                                                zoom: waveformTab.defaultZoom
                                            }
                                            WaveformMaskComponent {
                                                anchors.fill: parent
                                                rimColor: control.highlighted ? "#575757" : "#000000"
                                            }
                                        }
                                        Text {
                                            Layout.bottomMargin: 5
                                            Layout.leftMargin: 5
                                            color: "#FFFFFF"
                                            text: waveformDropdown.model[index]
                                        }
                                    }
                                }

                                // Text {
                                //     leftPadding: 5
                                //     rightPadding: waveformDropdown.indicator.width + waveformDropdown.spacing
                                //     text: waveformDropdown.displayText
                                //     font: waveformDropdown.font
                                //     color: Theme.deckTextColor
                                //     verticalAlignment: Text.AlignVCenter
                                //     elide: waveformDropdown.clip ? Text.ElideNone : Text.ElideRight
                                //     clip: waveformDropdown.clip
                                // }

                                popup: Popup {
                                    id: popup

                                    implicitHeight: contentItem.implicitHeight
                                    width: waveformDropdown.width - 27
                                    x: waveformDropdown.x + 27
                                    y: waveformDropdown.height

                                    background: Rectangle {
                                        border.width: 0
                                        color: '#000000'
                                        radius: 8
                                    }
                                    contentItem: ListView {
                                        anchors.fill: parent
                                        clip: true
                                        currentIndex: waveformDropdown.highlightedIndex
                                        implicitHeight: contentHeight
                                        model: waveformDropdown.popup.visible ? waveformDropdown.delegateModel : null

                                        ScrollIndicator.vertical: ScrollIndicator {
                                        }
                                    }
                                }

                                onActivated: selectedIndex => {
                                    currentIndex = selectedIndex;
                                }
                                onCurrentIndexChanged: waveformTab.dirty = true

                                Timer {
                                    interval: 25
                                    repeat: true
                                    running: !waveformDropdown.player?.isLoaded && waveformDropdown.track !== null

                                    onTriggered: {
                                        waveformDropdown.playbackProgress += interval / 1000 / waveformDropdown.track.duration;
                                        if (waveformDropdown.playbackProgress > 0.8) {
                                            waveformDropdown.playbackProgress = 0.2;
                                        }
                                    }
                                }
                            }
                            SettingComponents.Slider {
                                id: playMarkerPositionInput

                                Layout.fillWidth: true
                                Layout.preferredHeight: 23
                                Layout.topMargin: 15
                                markers: [0, 50, 100]
                                objectName: "setting_playMarkerPosition"
                                suffix: "%"
                                value: 50

                                onValueChanged: waveformTab.dirty = true
                            }
                            Text {
                                Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
                                Layout.topMargin: 10
                                color: "#D9D9D9"
                                font.pixelSize: 14
                                text: "Play marker position"
                            }
                        }
                    }
                    Mixxx.SettingGroup {
                        Layout.bottomMargin: 6
                        Layout.fillWidth: true
                        Layout.topMargin: 40
                        implicitHeight: playMarkerColumn.height
                        label: "Play marker hints"

                        Column {
                            id: playMarkerColumn

                            anchors.left: parent.left
                            anchors.right: parent.right

                            Text {
                                color: Theme.white
                                font.pixelSize: 14
                                font.weight: Font.DemiBold
                                text: "Play marker hints"
                            }
                            Item {
                                anchors.left: parent.left
                                anchors.right: parent.right
                                height: 10
                            }
                            Rectangle {
                                anchors.left: parent.left
                                anchors.right: parent.right
                                color: Theme.darkGray2
                                implicitHeight: playMarkerHintsPane.implicitHeight + 20

                                GridLayout {
                                    id: playMarkerHintsPane

                                    anchors.bottomMargin: 10
                                    anchors.fill: parent
                                    anchors.leftMargin: 17
                                    anchors.rightMargin: 17
                                    anchors.topMargin: 10
                                    columnSpacing: 20
                                    columns: 2
                                    rowSpacing: 8

                                    RowLayout {
                                        Layout.preferredWidth: (playMarkerHintsPane.width - 34) * 0.5

                                        Mixxx.SettingParameter {
                                            Layout.fillWidth: true
                                            label: "Beats until next marker"

                                            Text {
                                                anchors.fill: parent
                                                color: Theme.white
                                                font.pixelSize: 14
                                                font.weight: Font.Medium
                                                horizontalAlignment: Text.AlignLeft
                                                text: parent.label
                                                verticalAlignment: Text.AlignVCenter
                                            }
                                        }
                                        RatioChoice {
                                            id: untilMarkShowBeatsInput

                                            objectName: "setting_untilMarkShowBeats"

                                            readonly property bool isOn: selected == "on"

                                            function load(value) {
                                                untilMarkShowBeatsInput.selected = value ? "on" : "off";
                                            }

                                            inactiveColor: Theme.darkGray4
                                            options: ["on", "off"]

                                            onSelectedChanged: waveformTab.dirty = true
                                        }
                                    }
                                    RowLayout {
                                        Layout.preferredWidth: (playMarkerHintsPane.width - 34) * 0.5

                                        Mixxx.SettingParameter {
                                            Layout.fillWidth: true
                                            label: "Placement"

                                            Text {
                                                anchors.fill: parent
                                                color: Theme.white
                                                font.pixelSize: 14
                                                font.weight: Font.Medium
                                                horizontalAlignment: Text.AlignLeft
                                                text: parent.label
                                                verticalAlignment: Text.AlignVCenter
                                            }
                                        }
                                        RatioChoice {
                                            id: untilMarkAlignInput

                                            objectName: "setting_untilMarkAlign"

                                            function load(value) {
                                                switch (value) {
                                                case 0:
                                                    untilMarkAlignInput.selected = "top";
                                                    break;
                                                case 2:
                                                    untilMarkAlignInput.selected = "bottom";
                                                    break;
                                                default:
                                                    console.warn(`Unrecognised value '${value}' for Waveform,UntilMarkAlign. Defaulting to center (1).`);
                                                case 1:
                                                    untilMarkAlignInput.selected = "center";
                                                    break;
                                                }
                                            }

                                            options: ["top", "center", "bottom"]
                                            selected: "center"

                                            onSelectedChanged: waveformTab.dirty = true
                                        }
                                    }
                                    RowLayout {
                                        Layout.preferredWidth: (playMarkerHintsPane.width - 34) * 0.5

                                        Mixxx.SettingParameter {
                                            Layout.fillWidth: true
                                            label: "Time until next marker"

                                            Text {
                                                anchors.fill: parent
                                                color: Theme.white
                                                font.pixelSize: 14
                                                font.weight: Font.Medium
                                                horizontalAlignment: Text.AlignLeft
                                                text: parent.label
                                                verticalAlignment: Text.AlignVCenter
                                            }
                                        }
                                        RatioChoice {
                                            id: untilMarkShowTimeInput

                                            objectName: "setting_untilMarkShowTime"

                                            readonly property bool isOn: selected == "on"

                                            function load(value) {
                                                untilMarkShowTimeInput.selected = value ? "on" : "off";
                                            }

                                            inactiveColor: Theme.darkGray4
                                            options: ["on", "off"]

                                            onSelectedChanged: waveformTab.dirty = true
                                        }
                                    }
                                    RowLayout {
                                        Layout.preferredWidth: (playMarkerHintsPane.width - 34) * 0.5

                                        Mixxx.SettingParameter {
                                            Layout.fillWidth: true
                                            label: "Font size"

                                            Text {
                                                anchors.fill: parent
                                                color: Theme.white
                                                font.pixelSize: 14
                                                font.weight: Font.Medium
                                                horizontalAlignment: Text.AlignLeft
                                                text: parent.label
                                                verticalAlignment: Text.AlignVCenter
                                            }
                                        }
                                        Skin.ComboBox {
                                            id: untilMarkTextPointSizeInput

                                            objectName: "setting_untilMarkTextPointSize"

                                            property var fontSizes: [10, 12, 15, 18, 24, 32, 48]
                                            readonly property int value: fontSizes[currentIndex] ?? 10

                                            model: fontSizes.map(i => `${i} pt`)

                                            onCurrentIndexChanged: waveformTab.dirty = true
                                        }
                                    }
                                }
                            }
                        }
                    }
                    Mixxx.SettingGroup {
                        Layout.bottomMargin: 6
                        Layout.fillWidth: true
                        Layout.topMargin: 40
                        implicitHeight: visualGainColumn.height
                        label: "Visual gain"

                        Column {
                            id: visualGainColumn

                            anchors.left: parent.left
                            anchors.right: parent.right

                            RowLayout {
                                anchors.left: parent.left
                                anchors.right: parent.right

                                Text {
                                    Layout.fillWidth: true
                                    color: Theme.white
                                    font.pixelSize: 14
                                    font.weight: Font.DemiBold
                                    text: "Visual gain"
                                }
                                Text {
                                    color: Theme.white
                                    font.pixelSize: 14
                                    font.weight: Font.Medium
                                    horizontalAlignment: Text.AlignLeft
                                    text: "Global"
                                    verticalAlignment: Text.AlignVCenter
                                }
                                SettingComponents.Slider {
                                    id: visualGainAllInput

                                    markers: [50, 100, 150, 300, 500]
                                    min: 50
                                    max: 500
                                    objectName: "setting_visualGainAll"
                                    suffix: "%"
                                    value: 100
                                    width: 400

                                    onValueChanged: waveformTab.dirty = true
                                }
                            }
                            Item {
                                anchors.left: parent.left
                                anchors.right: parent.right
                                height: 10
                            }
                            Rectangle {
                                anchors.left: parent.left
                                anchors.right: parent.right
                                color: Theme.darkGray2
                                implicitHeight: visualGainPane.implicitHeight + 56

                                RowLayout {
                                    id: visualGainPane

                                    anchors.bottomMargin: 28
                                    anchors.fill: parent
                                    anchors.leftMargin: 17
                                    anchors.rightMargin: 17
                                    anchors.topMargin: 28
                                    spacing: 50

                                    GridLayout {
                                        Layout.preferredWidth: visualGainPane.width * 0.33
                                        columns: waveformTab.width < 1000 ? 2 : 3

                                        Mixxx.SettingParameter {
                                            Layout.fillWidth: waveformTab.width >= 1000
                                            Layout.preferredWidth: waveformTab.width < 1000 ? 100 : undefined
                                            Layout.column: 0
                                            Layout.row: 0

                                            label: "Low"

                                            Text {
                                                anchors.fill: parent
                                                color: Theme.white
                                                font.pixelSize: 14
                                                font.weight: Font.Medium
                                                horizontalAlignment: waveformTab.width < 1000 ? undefined : Text.AlignHCenter
                                                text: parent.label
                                                verticalAlignment: Text.AlignVCenter
                                            }
                                        }
                                        SettingComponents.Slider {
                                            id: visualGainLowInput

                                            Layout.fillWidth: true
                                            Layout.column: waveformTab.width < 1000 ? 1 : 0
                                            Layout.row: waveformTab.width < 1000 ? 0 : 1

                                            objectName: "setting_visualGainLow"
                                            markers: [50, 100, 150, 300, 500]
                                            min: 50
                                            max: 500
                                            suffix: "%"
                                            value: 100

                                            onValueChanged: waveformTab.dirty = true
                                        }
                                        Mixxx.SettingParameter {
                                            Layout.fillWidth: waveformTab.width >= 1000
                                            Layout.preferredWidth: waveformTab.width < 1000 ? 100 : undefined
                                            label: "Middle"

                                            Layout.column: waveformTab.width < 1000 ? 0 : 1
                                            Layout.row: waveformTab.width < 1000 ? 1 : 0

                                            Text {
                                                anchors.fill: parent
                                                color: Theme.white
                                                font.pixelSize: 14
                                                font.weight: Font.Medium
                                                horizontalAlignment: waveformTab.width < 1000 ? undefined : Text.AlignHCenter
                                                text: parent.label
                                                verticalAlignment: Text.AlignVCenter
                                            }
                                        }
                                        SettingComponents.Slider {
                                            id: visualGainMediumInput

                                            objectName: "setting_visualGainMedium"

                                            Layout.fillWidth: true
                                            Layout.column: 1
                                            Layout.row: 1

                                            markers: [50, 100, 150, 300, 500]
                                            min: 50
                                            max: 500
                                            suffix: "%"
                                            value: 100

                                            onValueChanged: waveformTab.dirty = true
                                        }
                                        Mixxx.SettingParameter {
                                            Layout.fillWidth: waveformTab.width >= 1000
                                            Layout.preferredWidth: waveformTab.width < 1000 ? 100 : undefined
                                            Layout.column: waveformTab.width < 1000 ? 0 : 2
                                            Layout.row: waveformTab.width < 1000 ? 2 : 0

                                            label: "High"

                                            Text {
                                                anchors.fill: parent
                                                color: Theme.white
                                                font.pixelSize: 14
                                                font.weight: Font.Medium
                                                horizontalAlignment: waveformTab.width < 1000 ? undefined : Text.AlignHCenter
                                                text: parent.label
                                                verticalAlignment: Text.AlignVCenter
                                            }
                                        }
                                        SettingComponents.Slider {
                                            id: visualGainHighInput

                                            objectName: "setting_visualGainHigh"

                                            Layout.fillWidth: true
                                            Layout.column: waveformTab.width < 1000 ? 1 : 2
                                            Layout.row: waveformTab.width < 1000 ? 2 : 1

                                            markers: [50, 100, 150, 300, 500]
                                            min: 50
                                            max: 500
                                            suffix: "%"
                                            value: 100

                                            onValueChanged: waveformTab.dirty = true
                                        }
                                    }
                                }
                            }
                        }
                    }
                    Item {
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                    }
                }
                // Test stand-in for the native ColorDialog the swatches open,
                // following the pattern of the library category's folder
                // picker mock: while testMode is on, open() increments the
                // open count and immediately accepts the color the test set
                // beforehand instead of showing the native dialog.
                Item {
                    id: colorPickerTest

                    objectName: "colorPickerTest"
                    visible: false
                    width: 0
                    height: 0

                    property bool testMode: false
                    property int openCount: 0
                    property string selectedColor: ""

                    signal accepted(int index)

                    function open(index) {
                        openCount++;
                        if (testMode && selectedColor.length > 0) {
                            accepted(index);
                        }
                    }
                    onAccepted: index => {
                        const color = colorPickerTest.selectedColor;
                        if (index === 0) {
                            waveformDropdown.lowColor = Qt.color(color);
                        } else if (index === 1) {
                            waveformDropdown.midColor = Qt.color(color);
                        } else {
                            waveformDropdown.highColor = Qt.color(color);
                        }
                    }
                }
            }
            Mixxx.SettingGroup {
                id: decksTab

                property bool dirty: false

                width: scrollView.width
                implicitHeight: decksTabColumn.y + decksTabColumn.height

                label: "Decks"
                visible: root.selectedIndex == 2

                onActivated: {
                    root.selectedIndex = 2;
                }

                ColumnLayout {
                    y: 20
                    id: decksTabColumn
                    width: decksTab.width
                    spacing: 0

                    GridLayout {
                        id: deckPane

                        columnSpacing: 20
                        columns: 2
                        // anchors.fill: parent
                        // anchors.bottomMargin: 10
                        // anchors.leftMargin: 3
                        // anchors.rightMargin: 3
                        rowSpacing: 15

                        RowLayout {
                            Layout.leftMargin: 14
                            Layout.preferredWidth: (deckPane.width - 56) * 0.5
                            Layout.rightMargin: 14

                            Mixxx.SettingParameter {
                                Layout.fillWidth: true
                                label: "Cue mode"

                                Text {
                                    anchors.fill: parent
                                    color: Theme.white
                                    font.pixelSize: 14
                                    font.weight: Font.Medium
                                    horizontalAlignment: Text.AlignLeft
                                    text: parent.label
                                    verticalAlignment: Text.AlignVCenter
                                }
                            }
                            Skin.ComboBox {
                                id: cueModeInput

                                implicitWidth: 200
                                model: ["Mixxx", "Mixxx (no blinking)", "Pioneer", "Denon", "Numark", "CUP",]
                                objectName: "setting_cueMode"

                                onCurrentIndexChanged: decksTab.dirty = true
                            }
                        }
                        RowLayout {
                            Layout.preferredWidth: (deckPane.width - 56) * 0.5
                            Layout.rightMargin: 14

                            Mixxx.SettingParameter {
                                Layout.fillWidth: true
                                label: "Time format"

                                Text {
                                    anchors.fill: parent
                                    color: Theme.white
                                    font.pixelSize: 14
                                    font.weight: Font.Medium
                                    horizontalAlignment: Text.AlignLeft
                                    text: parent.label
                                    verticalAlignment: Text.AlignVCenter
                                }
                            }
                            Skin.ComboBox {
                                id: timeMode

                                Layout.preferredWidth: (deckPane.width - 56) * 0.3
                                model: [DeckComponents.TrackTime.Mode.Traditional, DeckComponents.TrackTime.Mode.TraditionalCoarse, DeckComponents.TrackTime.Mode.Seconds, DeckComponents.TrackTime.Mode.SecondsLong, DeckComponents.TrackTime.Mode.KiloSeconds, DeckComponents.TrackTime.Mode.HectoSeconds,]
                                objectName: "setting_timeFormat"

                                contentItem: Content {
                                    anchors.fill: parent
                                    anchors.leftMargin: 10
                                    anchors.rightMargin: 20
                                    mode: timeMode.model[timeMode.currentIndex]
                                }
                                delegate: ItemDelegate {
                                    id: itemDlgt

                                    required property int index

                                    highlighted: root.highlightedIndex === this.index
                                    objectName: "setting_timeFormat_option_" + index
                                    text: content.text
                                    width: parent.width

                                    background: Rectangle {
                                        border.color: Theme.deckLineColor
                                        border.width: itemDlgt.highlighted ? 1 : 0
                                        color: "transparent"
                                        radius: 5
                                    }
                                    contentItem: Content {
                                        id: content

                                        anchors.fill: parent
                                        anchors.leftMargin: 10
                                        anchors.rightMargin: 20
                                        mode: timeMode.model[index]
                                    }
                                }

                                onCurrentIndexChanged: decksTab.dirty = true
                            }
                        }
                        RowLayout {
                            Layout.leftMargin: 14
                            Layout.preferredWidth: (deckPane.width - 56) * 0.5
                            Layout.rightMargin: 14

                            Mixxx.SettingParameter {
                                Layout.fillWidth: true
                                label: "Set intro start to main cue when analyzing tracks"

                                Text {
                                    anchors.fill: parent
                                    color: Theme.white
                                    font.pixelSize: 14
                                    font.weight: Font.Medium
                                    horizontalAlignment: Text.AlignLeft
                                    text: parent.label
                                    verticalAlignment: Text.AlignVCenter
                                }
                            }
                            RatioChoice {
                                id: setIntroStartToMainCueInput

                                readonly property bool isOn: selected == "on"

                                objectName: "setting_introStartToMainCue"
                                options: ["on", "off"]

                                onSelectedChanged: decksTab.dirty = true
                            }
                        }
                        RowLayout {
                            Layout.preferredWidth: (deckPane.width - 56) * 0.5
                            Layout.rightMargin: 14

                            Mixxx.SettingParameter {
                                Layout.fillWidth: true
                                label: "Track time display"

                                Text {
                                    anchors.fill: parent
                                    color: Theme.white
                                    font.pixelSize: 14
                                    font.weight: Font.Medium
                                    horizontalAlignment: Text.AlignLeft
                                    text: parent.label
                                    verticalAlignment: Text.AlignVCenter
                                }
                            }
                            RatioChoice {
                                id: trackTimeDisplayInput

                                maxWidth: deckPane.width * 0.28
                                objectName: "setting_trackTimeDisplay"
                                options: ["elapsed", "remaining", "both"]
                                normalizedWidth: false

                                onSelectedChanged: decksTab.dirty = true
                            }
                        }
                        RowLayout {
                            Layout.leftMargin: 14
                            Layout.preferredWidth: (deckPane.width - 56) * 0.5
                            Layout.rightMargin: 14

                            Mixxx.SettingParameter {
                                Layout.fillWidth: true
                                label: "Double-press Load button to clone playing track"

                                Text {
                                    anchors.fill: parent
                                    color: Theme.white
                                    font.pixelSize: 14
                                    font.weight: Font.Medium
                                    horizontalAlignment: Text.AlignLeft
                                    text: parent.label
                                    verticalAlignment: Text.AlignVCenter
                                }
                            }
                            RatioChoice {
                                id: doublePressLoadToCloneInput

                                objectName: "setting_doublePressLoadToClone"
                                options: ["on", "off"]

                                onSelectedChanged: decksTab.dirty = true
                            }
                        }
                        RowLayout {
                            Layout.preferredWidth: (deckPane.width - 56) * 0.5
                            Layout.rightMargin: 14

                            Mixxx.SettingParameter {
                                Layout.fillWidth: true
                                label: "Track load point"

                                Text {
                                    anchors.fill: parent
                                    color: Theme.white
                                    font.pixelSize: 14
                                    font.weight: Font.Medium
                                    horizontalAlignment: Text.AlignLeft
                                    text: parent.label
                                    verticalAlignment: Text.AlignVCenter
                                }
                            }
                            Skin.ComboBox {
                                id: trackLoadPointInput

                                model: ["Main cue", "Beginning of track", "First sound", "Intro start", "First hotcue"]
                                objectName: "setting_trackLoadPoint"
                                popupWidth: 130

                                onCurrentIndexChanged: decksTab.dirty = true
                            }
                        }
                        RowLayout {
                            Layout.leftMargin: 14
                            Layout.preferredWidth: (deckPane.width - 56) * 0.5
                            Layout.rightMargin: 14

                            Mixxx.SettingParameter {
                                Layout.fillWidth: true
                                label: "Loading a track when playing"

                                Text {
                                    anchors.fill: parent
                                    color: Theme.white
                                    font.pixelSize: 14
                                    font.weight: Font.Medium
                                    horizontalAlignment: Text.AlignLeft
                                    text: parent.label
                                    verticalAlignment: Text.AlignVCenter
                                }
                            }
                            RatioChoice {
                                id: loadingTrackWhenPlayingInput
                                maxWidth: deckPane.width * 0.28

                                objectName: "setting_loadingTrackWhenPlaying"
                                options: ["reject", "allow", "when stopped",]
                                normalizedWidth: false

                                onSelectedChanged: decksTab.dirty = true
                            }
                        }
                    }
                    Mixxx.SettingGroup {
                        Layout.bottomMargin: 6
                        Layout.fillWidth: true
                        Layout.topMargin: 40
                        implicitHeight: speedKeyColumn.height
                        label: "Speed & Key"

                        Column {
                            id: speedKeyColumn

                            anchors.left: parent.left
                            anchors.right: parent.right

                            Text {
                                anchors.left: parent.left
                                anchors.leftMargin: 17
                                color: Theme.white
                                font.pixelSize: 14
                                font.weight: Font.DemiBold
                                text: "Speed & Key"
                            }
                            Text {
                                anchors.left: parent.left
                                anchors.leftMargin: 17
                                color: Theme.white
                                font.pixelSize: 11
                                font.weight: Font.Thin
                                text: "or tempo & pitch"
                            }
                            Item {
                                anchors.left: parent.left
                                anchors.right: parent.right
                                height: 10
                            }
                            Rectangle {
                                anchors.left: parent.left
                                anchors.right: parent.right
                                color: Theme.darkGray3
                                implicitHeight: speedKeyPane.implicitHeight + 20

                                GridLayout {
                                    id: speedKeyPane

                                    anchors.bottomMargin: 10
                                    anchors.fill: parent
                                    anchors.leftMargin: 17
                                    anchors.rightMargin: 17
                                    anchors.topMargin: 10
                                    columnSpacing: 20
                                    columns: 2
                                    rowSpacing: 15

                                    RowLayout {
                                        Layout.preferredWidth: speedKeyPane.width * 0.5

                                        Mixxx.SettingParameter {
                                            Layout.fillWidth: true
                                            label: "Reset on track load"

                                            Text {
                                                anchors.fill: parent
                                                color: Theme.white
                                                font.pixelSize: 14
                                                font.weight: Font.Medium
                                                horizontalAlignment: Text.AlignLeft
                                                text: parent.label
                                                verticalAlignment: Text.AlignVCenter
                                            }
                                        }
                                        RatioChoice {
                                            id: resetOnTrackLoadInput

                                            maxWidth: deckPane.width * 0.38
                                            objectName: "setting_resetOnTrackLoad"
                                            options: ["none", "key", "both", "tempo",]

                                            onSelectedChanged: decksTab.dirty = true
                                        }
                                    }
                                    RowLayout {
                                        Layout.preferredWidth: speedKeyPane.width * 0.5

                                        Mixxx.SettingParameter {
                                            Layout.fillWidth: true
                                            label: "Slider range"

                                            Text {
                                                anchors.fill: parent
                                                color: Theme.white
                                                font.pixelSize: 14
                                                font.weight: Font.Medium
                                                horizontalAlignment: Text.AlignLeft
                                                text: parent.label
                                                verticalAlignment: Text.AlignVCenter
                                            }
                                        }
                                        Skin.ComboBox {
                                            id: sliderRangeInput

                                            readonly property list<int> values: [4, 6, 8, 10, 16, 24, 50, 90]

                                            model: ["4%", "6% (semitone)", "8% (Technics SL-1210)", "10%", "16%", "24%", "50%", "90%"]
                                            objectName: "setting_sliderRange"
                                            popupWidth: 150

                                            onCurrentIndexChanged: decksTab.dirty = true
                                        }
                                    }
                                    RowLayout {
                                        Layout.preferredWidth: speedKeyPane.width * 0.5

                                        Mixxx.SettingParameter {
                                            Layout.fillWidth: true
                                            label: "Sync mode"

                                            Text {
                                                anchors.fill: parent
                                                color: Theme.white
                                                font.pixelSize: 14
                                                font.weight: Font.Medium
                                                horizontalAlignment: Text.AlignLeft
                                                text: parent.label
                                                verticalAlignment: Text.AlignVCenter
                                            }
                                        }
                                        RatioChoice {
                                            id: syncModeInput

                                            maxWidth: deckPane.width * 0.38
                                            objectName: "setting_syncMode"
                                            options: ["follow soft leader", "use steady"]
                                            normalizedWidth: false

                                            onSelectedChanged: decksTab.dirty = true
                                        }
                                    }
                                    RowLayout {
                                        Layout.preferredWidth: speedKeyPane.width * 0.5

                                        Mixxx.SettingParameter {
                                            Layout.fillWidth: true
                                            height: 32
                                            label: "Slider orientation"

                                            Column {
                                                anchors.fill: parent

                                                Text {
                                                    color: Theme.white
                                                    font.pixelSize: 14
                                                    font.weight: Font.Medium
                                                    text: "Slider orientation"
                                                }
                                                Text {
                                                    color: Theme.white
                                                    font.italic: true
                                                    font.pixelSize: 11
                                                    font.weight: Font.Thin
                                                    text: "Define which end of the slider will increase the pitch"
                                                }
                                            }
                                        }
                                        RatioChoice {
                                            id: sliderOrientationInput

                                            maxWidth: deckPane.width * 0.38
                                            objectName: "setting_sliderOrientation"
                                            options: ["down", "up"]

                                            onSelectedChanged: decksTab.dirty = true
                                        }
                                    }
                                    RowLayout {
                                        Layout.preferredWidth: speedKeyPane.width * 0.5

                                        Mixxx.SettingParameter {
                                            Layout.fillWidth: true
                                            label: "Keylock mode"

                                            Text {
                                                anchors.fill: parent
                                                color: Theme.white
                                                font.pixelSize: 14
                                                font.weight: Font.Medium
                                                horizontalAlignment: Text.AlignLeft
                                                text: parent.label
                                                verticalAlignment: Text.AlignVCenter
                                            }
                                        }
                                        RatioChoice {
                                            id: keylockModeInput

                                            maxWidth: deckPane.width * 0.34
                                            objectName: "setting_keylockMode"
                                            options: ["original key", "current key"]
                                            normalizedWidth: false

                                            onSelectedChanged: decksTab.dirty = true
                                        }
                                    }
                                    RowLayout {
                                        Layout.preferredWidth: speedKeyPane.width * 0.5

                                        Mixxx.SettingParameter {
                                            Layout.fillWidth: true
                                            label: "Keyunlock mode"

                                            Text {
                                                anchors.fill: parent
                                                color: Theme.white
                                                font.pixelSize: 14
                                                font.weight: Font.Medium
                                                horizontalAlignment: Text.AlignLeft
                                                text: parent.label
                                                verticalAlignment: Text.AlignVCenter
                                            }
                                        }
                                        RatioChoice {
                                            id: keyunlockModeInput

                                            maxWidth: deckPane.width * 0.3
                                            objectName: "setting_keyunlockMode"
                                            options: ["reset key", "keep key"]

                                            onSelectedChanged: decksTab.dirty = true
                                        }
                                    }
                                    RowLayout {
                                        Layout.preferredWidth: speedKeyPane.width * 0.5

                                        Mixxx.SettingParameter {
                                            Layout.fillWidth: true
                                            label: "Pitch bend behaviour"

                                            Text {
                                                anchors.fill: parent
                                                color: Theme.white
                                                font.pixelSize: 14
                                                font.weight: Font.Medium
                                                horizontalAlignment: Text.AlignLeft
                                                text: parent.label
                                                verticalAlignment: Text.AlignVCenter
                                            }
                                        }
                                        RatioChoice {
                                            id: pitchBendBehaviourInput

                                            maxWidth: deckPane.width * 0.38
                                            objectName: "setting_pitchBendBehaviour"
                                            options: ["abrupt jump", "smooth ramping",]

                                            onSelectedChanged: decksTab.dirty = true
                                        }
                                    }
                                    RowLayout {
                                        Layout.preferredWidth: speedKeyPane.width * 0.5
                                        Layout.rowSpan: 3

                                        Mixxx.SettingParameter {
                                            label: "Pitch bend behaviour"
                                        }
                                        GridLayout {
                                            Layout.fillWidth: true
                                            Layout.rightMargin: 20
                                            columns: 3

                                            Text {
                                                // Layout.fillWidth: true
                                                Layout.alignment: Qt.AlignHCenter
                                                color: Theme.white
                                                font.pixelSize: 14
                                                font.weight: Font.DemiBold
                                                text: "Adjustment buttons"
                                            }
                                            Text {
                                                // Layout.fillWidth: true
                                                Layout.alignment: Qt.AlignHCenter
                                                color: Theme.white
                                                font.pixelSize: 14
                                                font.weight: Font.Medium
                                                text: "Fine"
                                            }
                                            Text {
                                                // Layout.fillWidth: true
                                                Layout.alignment: Qt.AlignHCenter
                                                color: Theme.white
                                                font.pixelSize: 14
                                                font.weight: Font.Medium
                                                text: "Coarse"
                                            }
                                            Text {
                                                Layout.leftMargin: 20
                                                color: Theme.white
                                                font.pixelSize: 14
                                                font.weight: Font.Medium
                                                text: "Temporary"
                                            }
                                            SettingComponents.SpinBox {
                                                id: adjustmentButtonsTemporaryFineInput

                                                Layout.alignment: Qt.AlignHCenter
                                                Layout.fillWidth: true
                                                Layout.leftMargin: 20
                                                Layout.rightMargin: 20
                                                max: 10
                                                min: 0.01
                                                objectName: "setting_temporaryFineAdjustment"
                                                precision: 2
                                                realValue: 2
                                                suffix: "%"

                                                onValueChanged: decksTab.dirty = true
                                            }
                                            SettingComponents.SpinBox {
                                                id: adjustmentButtonsTemporaryCoarseInput

                                                Layout.alignment: Qt.AlignHCenter
                                                Layout.fillWidth: true
                                                Layout.leftMargin: 20
                                                Layout.rightMargin: 20
                                                max: 10
                                                min: 0.01
                                                objectName: "setting_temporaryCoarseAdjustment"
                                                precision: 2
                                                realValue: 4
                                                suffix: "%"

                                                onValueChanged: decksTab.dirty = true
                                            }
                                            Text {
                                                Layout.leftMargin: 20
                                                color: Theme.white
                                                font.pixelSize: 14
                                                font.weight: Font.Medium
                                                text: "Permanent"
                                            }
                                            SettingComponents.SpinBox {
                                                id: adjustmentButtonsPermanentFineInput

                                                Layout.alignment: Qt.AlignHCenter
                                                Layout.fillWidth: true
                                                Layout.leftMargin: 20
                                                Layout.rightMargin: 20
                                                max: 10
                                                min: 0.01
                                                objectName: "setting_permanentFineAdjustment"
                                                precision: 2
                                                realValue: 0.05
                                                suffix: "%"

                                                onValueChanged: decksTab.dirty = true
                                            }
                                            SettingComponents.SpinBox {
                                                id: adjustmentButtonsPermanentCoarseInput

                                                Layout.alignment: Qt.AlignHCenter
                                                Layout.fillWidth: true
                                                Layout.leftMargin: 20
                                                Layout.rightMargin: 20
                                                max: 10
                                                min: 0.01
                                                objectName: "setting_permanentCoarseAdjustment"
                                                precision: 2
                                                realValue: 0.5
                                                suffix: "%"

                                                onValueChanged: decksTab.dirty = true
                                            }
                                        }
                                    }
                                    RowLayout {
                                        Layout.preferredWidth: speedKeyPane.width * 0.5

                                        Mixxx.SettingParameter {
                                            Layout.fillWidth: true
                                            label: "Ramping sensitivity"

                                            Text {
                                                anchors.fill: parent
                                                color: Theme.white
                                                font.pixelSize: 14
                                                font.weight: Font.Medium
                                                horizontalAlignment: Text.AlignLeft
                                                text: parent.label
                                                verticalAlignment: Text.AlignVCenter
                                            }
                                        }
                                        SettingComponents.Slider {
                                            id: rampingSensitivityInput

                                            max: 2500
                                            min: 100
                                            objectName: "setting_rampingSensitivity"
                                            value: 250
                                            wheelStep: 50
                                            width: 300

                                            onValueChanged: decksTab.dirty = true
                                        }
                                    }
                                }
                            }
                        }
                    }
                    Item {
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                    }
                }
            }
        }
    }
    Item {
        id: buttonActions

        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.leftMargin: 14
        anchors.right: parent.right
        anchors.rightMargin: 14
        height: 20

        Skin.FormButton {
            activeColor: "#999999"
            anchors.left: parent.left
            backgroundColor: "#7D3B3B"
            objectName: "interfaceResetButton"
            opacity: enabled ? 1.0 : 0.5
            text: "Reset"

            onPressed: {
                switch (root.selectedIndex) {
                case 0:
                    root.resetInterface();
                    break;
                case 1:
                    root.resetWaveform();
                    break;
                case 2:
                    root.resetDeck();
                    break;
                }
            }
        }
        Row {
            anchors.right: parent.right
            spacing: 10

            readonly property bool hasChanges: root.selectedIndex == 0 && themeColorTab.dirty || root.selectedIndex == 1 && waveformTab.dirty || root.selectedIndex == 2 && decksTab.dirty

            Text {
                id: errorMessage

                Layout.alignment: Qt.AlignVCenter
                Layout.rightMargin: 16
                color: "#7D3B3B"
                text: ""
            }
            Skin.FormButton {
                activeColor: "#999999"
                backgroundColor: "#3F3F3F"
                // Weird bug on MacOS - if the button is disabled before the pressed property is clicked, the button remains pressed, leading to a no-op click when it gets next enabled
                enabled: pressed || parent.hasChanges
                objectName: "interfaceCancelButton"
                opacity: enabled ? 1.0 : 0.5
                text: "Cancel"

                onPressed: {
                    switch (root.selectedIndex) {
                    case 0:
                        root.loadInterface();
                        break;
                    case 1:
                        root.loadWaveform();
                        break;
                    case 2:
                        root.loadDeck();
                        break;
                    }
                }
            }
            Skin.FormButton {
                activeColor: "#999999"
                backgroundColor: enabled ? "#3a60be" : "#3F3F3F"
                // Weird bug on MacOS - if the button is disabled before the pressed property is clicked, the button remains pressed, leading to a no-op click when it gets next enabled
                enabled: pressed || parent.hasChanges
                objectName: "interfaceSaveButton"
                opacity: enabled ? 1.0 : 0.5
                text: "Save"

                onPressed: {
                    errorMessage.text = "";
                    switch (root.selectedIndex) {
                    case 0:
                        root.saveInterface();
                        break;
                    case 1:
                        root.saveWaveform();
                        break;
                    case 2:
                        root.saveDeck();
                        break;
                    }
                }
            }
        }
    }

    component Content: RowLayout {
        required property var mode

        Text {
            Layout.fillWidth: true
            color: Theme.deckTextColor
            elide: Text.ElideRight
            text: {
                switch (parent.mode) {
                case DeckComponents.TrackTime.Mode.TraditionalCoarse:
                    return "Traditional (Coarse)";
                case DeckComponents.TrackTime.Mode.Seconds:
                    return "Seconds";
                case DeckComponents.TrackTime.Mode.SecondsLong:
                    return "Seconds (Long)";
                case DeckComponents.TrackTime.Mode.KiloSeconds:
                    return "Kiloseconds";
                case DeckComponents.TrackTime.Mode.HectoSeconds:
                    return "Hectoseconds";
                default:
                    console.warn(`Unsupported track time mode: ${root.mode}. Defaulting to traditional`);
                case DeckComponents.TrackTime.Mode.Traditional:
                    return "Traditional";
                }
            }
            verticalAlignment: Text.AlignVCenter
        }
        DeckComponents.TrackTime {
            Layout.rightMargin: 17
            display: trackTimeDisplayInput.selected === "elapsed" ? DeckComponents.TrackTime.Display.Elapsed : trackTimeDisplayInput.selected === "remaining" ? DeckComponents.TrackTime.Display.Remaining : DeckComponents.TrackTime.Display.Both
            elapsed: 83.45
            mode: parent.mode
            remaining: 147.23
        }
    }
}
