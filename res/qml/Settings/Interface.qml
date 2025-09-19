import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Dialogs
import Qt5Compat.GraphicalEffects
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
        sliderOrientationInput.selected = Mixxx.Config.controlRateDir === -1 ? "down" : "up";
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
        console.log(colorPane.hotcuePaletteColorIndex, Mixxx.Config.controlHotcueDefaultColorIndex);
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
    }
    function resetDeck() {
    }
    function resetInterface() {
    }
    function resetWaveform() {
    }
    function saveDeck() {
        Mixxx.Config.controlCueDefault = cueModeInput.currentIndex;
        Mixxx.Config.controlSetIntroStartAtMainCue = setIntroStartToMainCueInput.enabled;
        Mixxx.Config.controlPositionDisplay = trackTimeDisplayInput.options.indexOf(trackTimeDisplayInput.selected);
        Mixxx.Config.controlCloneDeckOnLoadDoubleTap = doublePressLoadToCloneInput.enabled;
        Mixxx.Config.controlCueRecall = trackLoadPointInput.currentIndex;
        Mixxx.Config.controlLoadWhenDeckPlaying = loadingTrackWhenPlayingInput.options.indexOf(loadingTrackWhenPlayingInput.selected);
        Mixxx.Config.controlSpeedAutoReset = resetOnTrackLoadInput.options.indexOf(resetOnTrackLoadInput.selected);
        for (let i = 0; i < deckRateRange.count; i++) {
            deckRateRange.objectAt(i).value = sliderRangeInput.values[sliderRangeInput.currentIndex];
        }
        Mixxx.Config.controlRateRange = sliderRangeInput.values[sliderRangeInput.currentIndex];
        Mixxx.Config.bpmSyncLockAlgorithm = syncModeInput.options.indexOf(syncModeInput.selected);
        for (let i = 0; i < deckRateDirection.count; i++) {
            deckRateDirection.objectAt(i).value = sliderOrientationInput.selected === "down" ? -1 : 1;
        }
        Mixxx.Config.controlRateDir = sliderOrientationInput.selected === "down" ? -1 : 1;
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
        Mixxx.Config.configStartInFullscreenKey = startFullscreenInput.enabled;
        Mixxx.Config.libraryHideMenuBar = autoHideMenuBarInput.enabled;
        Mixxx.Config.libraryEnableSearchCompletions = searchCompletionInput.enabled;
        Mixxx.Config.libraryEnableSearchHistoryShortcuts = searchHistoryKeyboardInput.enabled;
        Mixxx.Config.libraryBpmColumnPrecision = bpmPrecisionInput.value;
        Mixxx.Config.libraryRowHeight = libraryRowHeightInput.value;
        Mixxx.Config.configTrackColorPalette = trackPaletteComboBox.model[trackPaletteComboBox.currentIndex];
        Mixxx.Config.configHotcueColorPalette = hotcuePaletteComboBox.model[hotcuePaletteComboBox.currentIndex];
        Mixxx.Config.configKeyColorsEnabled = keyPaletteInput.enabled;
        Mixxx.Config.configKeyColorPalette = keyPaletteComboBox.model[keyPaletteComboBox.currentIndex];
        // keyPaletteInput.value
        Mixxx.Config.controlHotcueDefaultColorIndex = hotcuePaletteInput.currentIndex;
        Mixxx.Config.controlLoopDefaultColorIndex = loopPaletteInput.currentIndex;
        // jumpPaletteInput.value =
        loadInterface();
    }
    function saveWaveform() {
        loadWaveform();
    }

    label: "Interface"
    selectedIndex: 0
    tabs: ["theme & color", "waveform", "decks"]

    Component.onCompleted: {
        root.load();
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
    Item {
        anchors.bottom: buttonActions.top
        anchors.bottomMargin: 18
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top

        Mixxx.SettingGroup {
            id: themeColorTab

            property bool dirty: false

            anchors.fill: parent
            label: "Theme & Color"
            visible: root.selectedIndex == 0

            onActivated: {
                root.selectedIndex = 0;
            }

            ColumnLayout {
                anchors.fill: parent
                anchors.topMargin: 20
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
                                        inactiveColor: Theme.darkGray4
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
                                        inactiveColor: Theme.darkGray4
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
                                        precision: 0
                                        realValue: 1

                                        contentItem: Item {
                                            Rectangle {
                                                id: content

                                                anchors.fill: parent
                                                color: Theme.accentColor

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
                                            InnerShadow {
                                                id: bottomInnerEffect

                                                anchors.fill: parent
                                                color: "#0E2A54"
                                                horizontalOffset: -1
                                                radius: 8
                                                samples: 32
                                                source: content
                                                spread: 0.4
                                                verticalOffset: -1
                                            }
                                            InnerShadow {
                                                id: topInnerEffect

                                                anchors.fill: parent
                                                color: "#0E2A54"
                                                horizontalOffset: 1
                                                radius: 8
                                                samples: 32
                                                source: bottomInnerEffect
                                                spread: 0.4
                                                verticalOffset: 1
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
                                        suffix: "px"
                                        value: 14
                                        width: 400

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

                                        options: ["on", "off"]

                                        onSelectedChanged: themeColorTab.dirty = true
                                    }
                                    ColorPaletteComboBox {
                                        id: keyPaletteComboBox

                                        enabled: keyPaletteInput.enabled
                                        model: Mixxx.Config.paletteNames().filter(palette => Mixxx.Config.colorPalette(palette).length == 12)
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

            property bool dirty: false

            anchors.fill: parent
            label: "Waveform"
            visible: root.selectedIndex == 1

            onActivated: {
                root.selectedIndex = 1;
            }
        }
        Mixxx.SettingGroup {
            id: decksTab

            property bool dirty: false

            anchors.fill: parent
            label: "Decks"
            visible: root.selectedIndex == 2

            onActivated: {
                root.selectedIndex = 2;
            }

            ColumnLayout {
                anchors.fill: parent
                anchors.topMargin: 20
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

                            Layout.preferredWidth: (deckPane.width - 56) * 0.2
                            model: [DeckComponents.TrackTime.Mode.Traditional, DeckComponents.TrackTime.Mode.TraditionalCoarse, DeckComponents.TrackTime.Mode.Seconds, DeckComponents.TrackTime.Mode.SecondsLong, DeckComponents.TrackTime.Mode.KiloSeconds, DeckComponents.TrackTime.Mode.HectoSeconds,]

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

                            readonly property bool enabled: selected == "on"

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

                            options: ["elapsed", "remaining", "both"]

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

                            model: ["Main cue", "Beginning of track", "First sound", "Intro start", "First hotcue",]

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

                            options: ["reject", "allow", "when stopped",]

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
                            color: Theme.darkGray2
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

                                        inactiveColor: Theme.darkGray4
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

                                        inactiveColor: Theme.darkGray4
                                        options: ["follow soft leader", "use steady"]

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

                                        inactiveColor: Theme.darkGray4
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

                                        inactiveColor: Theme.darkGray4
                                        options: ["original key", "current key"]

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

                                        inactiveColor: Theme.darkGray4
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

                                        inactiveColor: Theme.darkGray4
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
                                        value: 250
                                        wheelStep: 50
                                        width: 400

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
    Item {
        id: buttonActions

        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.leftMargin: 14
        anchors.right: parent.right
        anchors.rightMargin: 14
        height: 20

        SettingComponents.FormButton {
            activeColor: "#999999"
            anchors.left: parent.left
            backgroundColor: "#7D3B3B"
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
                case 0:
                    root.resetDeck();
                    break;
                }
            }
        }
        Row {
            anchors.right: parent.right
            spacing: 10

            Text {
                id: errorMessage

                Layout.alignment: Qt.AlignVCenter
                Layout.rightMargin: 16
                color: "#7D3B3B"
                text: ""
            }
            SettingComponents.FormButton {
                activeColor: "#999999"
                backgroundColor: "#3F3F3F"
                enabled: root.selectedIndex == 0 && themeColorTab.dirty || root.selectedIndex == 1 && waveformTab.dirty || root.selectedIndex == 2 && decksTab.dirty
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
            SettingComponents.FormButton {
                activeColor: "#999999"
                backgroundColor: root.hasChanges ? "#3a60be" : "#3F3F3F"
                enabled: root.selectedIndex == 0 && themeColorTab.dirty || root.selectedIndex == 1 && waveformTab.dirty || root.selectedIndex == 2 && decksTab.dirty
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
