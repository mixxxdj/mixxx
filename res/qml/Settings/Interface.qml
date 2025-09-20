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
    tabs: ["theme & color", "waveform", "decks"]
    selectedIndex: 0

    label: "Interface"

    Component.onCompleted: {
        root.load()
    }

    onActivated: {
        waveformDropdown.track = waveformDropdown.player.isLoaded ? waveformDropdown.player.currentTrack : Mixxx.Library.model.getTrack(Mixxx.Library.model.length * Math.random())
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
            group: `[Channel${index+1}]`
            key: "rateRange"
        }
    }
    Instantiator {
        id: deckRateDirection
        model: numDecks.value
        Mixxx.ControlProxy {
            required property int index
            group: `[Channel${index+1}]`
            key: "rate_dir"
        }
    }

    Item {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: buttonActions.top
        anchors.bottomMargin: 18

        Mixxx.SettingGroup {
            property bool dirty: false
            id: themeColorTab
            label: "Theme & Color"
            visible: root.selectedIndex == 0
            anchors.fill: parent

            onActivated: {
                root.selectedIndex = 0;
            }
            ColumnLayout {
                anchors.fill: parent
                anchors.topMargin: 20
                spacing: 0
                RowLayout {
                    id: theme
                    spacing: 20
                    Layout.leftMargin: 14
                    Layout.rightMargin: 14
                    ColumnLayout {
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignTop
                        spacing: 15

                        RowLayout {

                            Mixxx.SettingParameter {
                                Layout.fillWidth: true
                                label: "Skin"
                                Text {
                                    text: parent.label
                                    color: Theme.white
                                    font.pixelSize: 14
                                }
                            }
                            Skin.ComboBox {
                                onCurrentIndexChanged: themeColorTab.dirty = true
                                id: skinInput
                                model: [
                                        "Unnamed"
                                ]
                            }
                        }

                        RowLayout {
                            Mixxx.SettingParameter {
                                Layout.fillWidth: true
                                label: "Color"
                                Text {
                                    text: parent.label
                                    color: Theme.white
                                    font.pixelSize: 14
                                }
                            }
                            Skin.ComboBox {
                                onCurrentIndexChanged: themeColorTab.dirty = true
                                id: colorInput
                                model: [
                                        "Dark",
                                        "Light"
                                ]
                            }
                        }

                        RowLayout {
                            Mixxx.SettingParameter {
                                Layout.fillWidth: true
                                label: "Layout"
                                Text {
                                    text: parent.label
                                    color: Theme.white
                                    font.pixelSize: 14
                                }
                            }
                            Skin.ComboBox {
                                onCurrentIndexChanged: themeColorTab.dirty = true
                                id: layoutInput
                                model: [
                                        "Performance",
                                        "Broadcast"
                                ]
                            }
                        }

                        RowLayout {
                            Mixxx.SettingParameter {
                                Layout.fillWidth: true
                                label: "Tool tips"
                                Text {
                                    text: parent.label
                                    color: Theme.white
                                    font.pixelSize: 14
                                }
                            }
                            RatioChoice {
                                onSelectedChanged: themeColorTab.dirty = true
                                id: tooltipsInput
                                options: [
                                          "off",
                                          "library",
                                          "all"
                                ]
                            }
                        }

                        RowLayout {
                            Mixxx.SettingParameter {
                                Layout.fillWidth: true
                                label: "Disable screen saver"
                                Text {
                                    text: parent.label
                                    color: Theme.white
                                    font.pixelSize: 14
                                }
                            }
                            RatioChoice {
                                onSelectedChanged: themeColorTab.dirty = true
                                id: disableScreensaverInput
                                options: [
                                          "no",
                                          "while running",
                                          "while playing"
                                ]
                            }
                        }

                        RowLayout {
                            Mixxx.SettingParameter {
                                Layout.fillWidth: true
                                label: "Start in full-screen mode"
                                Text {
                                    text: parent.label
                                    color: Theme.white
                                    font.pixelSize: 14
                                }
                            }
                            RatioChoice {
                                onSelectedChanged: themeColorTab.dirty = true
                                id: startFullscreenInput
                                readonly property bool enabled: selected == "on"
                                options: [
                                          "on",
                                          "off"
                                ]
                            }
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            ColumnLayout {
                                Layout.fillWidth: true
                                Mixxx.SettingParameter {
                                    Layout.fillWidth: true
                                    label: "Auto-hide the menu bar"
                                    height: 14
                                    Text {
                                        text: parent.label
                                        color: Theme.white
                                        font.pixelSize: 14
                                    }
                                }
                                Text {
                                    text: "Toggle it with a single Alt key press"
                                    color: Theme.white
                                    font.pixelSize: 12
                                    font.italic: true
                                    font.weight: Font.Thin
                                }
                            }
                            RatioChoice {
                                onSelectedChanged: themeColorTab.dirty = true
                                id: autoHideMenuBarInput
                                readonly property bool enabled: selected == "on"
                                options: [
                                          "on",
                                          "off"
                                ]
                            }
                        }
                    }
                    Item {
                        Layout.preferredWidth: root.width * 0.35
                        Rectangle {
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.horizontalCenter: parent.horizontalCenter
                            width: parent.width - 160
                            height: width / 16 * 9
                            color: '#343434'
                            Skin.Button {
                                anchors.horizontalCenter: parent.horizontalCenter
                                anchors.bottom: parent.bottom
                                anchors.bottomMargin: 20
                                width: 80
                                activeColor: Theme.white
                                text: "Customize"
                            }
                        }
                    }
                }
                Mixxx.SettingGroup {
                    Layout.topMargin: 40
                    Layout.bottomMargin: 6
                    label: "Library"
                    implicitHeight: libraryColumn.height
                    Layout.fillWidth: true
                    Column {
                        id: libraryColumn
                        anchors.left: parent.left
                        anchors.right: parent.right
                        Text {
                            text: "Library"
                            color: Theme.white
                            font.pixelSize: 14
                            font.weight: Font.DemiBold
                        }
                        Item {
                            anchors.left: parent.left
                            anchors.right: parent.right
                            height: 10
                        }
                        Rectangle {
                            color: Theme.darkGray2
                            implicitHeight: libraryPane.implicitHeight + 20
                            anchors.left: parent.left
                            anchors.right: parent.right
                            GridLayout {
                                id: libraryPane
                                anchors.fill: parent
                                anchors.topMargin: 10
                                anchors.bottomMargin: 10
                                anchors.leftMargin: 17
                                anchors.rightMargin: 17
                                rowSpacing: 15
                                columnSpacing: 20
                                columns: 2

                                RowLayout {
                                    Layout.preferredWidth: libraryPane.width * 0.5
                                    Mixxx.SettingParameter {
                                        Layout.fillWidth: true
                                        label: "Search completion"
                                        Text {
                                            anchors.fill: parent

                                            horizontalAlignment: Text.AlignLeft
                                            verticalAlignment: Text.AlignVCenter

                                            text: parent.label
                                            color: Theme.white
                                            font.pixelSize: 14
                                            font.weight: Font.Medium
                                        }
                                    }
                                    RatioChoice {
                                        onSelectedChanged: themeColorTab.dirty = true
                                        id: searchCompletionInput
                                        inactiveColor: Theme.darkGray4
                                        readonly property bool enabled: selected == "on"
                                        options: [
                                                  "on",
                                                  "off"
                                        ]
                                    }
                                }

                                RowLayout {
                                    Layout.preferredWidth: libraryPane.width * 0.5
                                    Mixxx.SettingParameter {
                                        Layout.fillWidth: true
                                        label: "Search history keyboard shortcuts"
                                        Text {
                                            anchors.fill: parent

                                            horizontalAlignment: Text.AlignLeft
                                            verticalAlignment: Text.AlignVCenter

                                            text: parent.label
                                            color: Theme.white
                                            font.pixelSize: 14
                                            font.weight: Font.Medium
                                        }
                                    }
                                    RatioChoice {
                                        onSelectedChanged: themeColorTab.dirty = true
                                        id: searchHistoryKeyboardInput
                                        inactiveColor: Theme.darkGray4
                                        readonly property bool enabled: selected == "on"
                                        options: [
                                                  "on",
                                                  "off"
                                        ]
                                    }
                                }

                                RowLayout {
                                    Layout.preferredWidth: libraryPane.width * 0.5
                                    Mixxx.SettingParameter {
                                        Layout.fillWidth: true
                                        label: "BPM display precision"
                                        Text {
                                            anchors.fill: parent

                                            horizontalAlignment: Text.AlignLeft
                                            verticalAlignment: Text.AlignVCenter

                                            text: parent.label
                                            color: Theme.white
                                            font.pixelSize: 14
                                            font.weight: Font.Medium
                                        }
                                    }
                                    SettingComponents.SpinBox {
                                        onValueChanged: themeColorTab.dirty = true
                                        id: bpmPrecisionInput
                                        realValue: 1
                                        precision: 0
                                        min: 0
                                        max: 10
                                        editable: false
                                        implicitWidth: 180

                                        contentItem: Item {
                                            Rectangle {
                                                id: content
                                                anchors.fill: parent
                                                color: Theme.accentColor
                                                Text {
                                                    id: textLabel
                                                    anchors.fill: parent
                                                    text: (126.0).toFixed(bpmPrecisionInput.value)
                                                    color: Theme.white
                                                    font: bpmPrecisionInput.font
                                                    horizontalAlignment: Text.AlignHCenter
                                                    verticalAlignment: Text.AlignVCenter
                                                }
                                            }
                                            InnerShadow {
                                                id: bottomInnerEffect
                                                anchors.fill: parent
                                                radius: 8
                                                samples: 32
                                                spread: 0.4
                                                horizontalOffset: -1
                                                verticalOffset: -1
                                                color: "#0E2A54"
                                                source: content
                                            }
                                            InnerShadow {
                                                id: topInnerEffect
                                                anchors.fill: parent
                                                radius: 8
                                                samples: 32
                                                spread: 0.4
                                                horizontalOffset: 1
                                                verticalOffset: 1
                                                color: "#0E2A54"
                                                source: bottomInnerEffect
                                            }
                                        }
                                    }
                                }

                                RowLayout {
                                    Layout.preferredWidth: libraryPane.width * 0.5
                                    Mixxx.SettingParameter {
                                        Layout.fillWidth: true
                                        label: "Library Row Height"
                                        Text {
                                            anchors.fill: parent

                                            horizontalAlignment: Text.AlignLeft
                                            verticalAlignment: Text.AlignVCenter

                                            text: parent.label
                                            color: Theme.white
                                            font.pixelSize: 14
                                            font.weight: Font.Medium
                                        }
                                    }
                                    SettingComponents.Slider {
                                        onValueChanged: themeColorTab.dirty = true
                                        id: libraryRowHeightInput
                                        width: 400
                                        markers: [14, 20, 50, 80]
                                        suffix: "px"
                                        value: 14
                                        min: 14
                                        max: 100
                                    }
                                }
                            }
                        }
                    }
                }
                Mixxx.SettingGroup {
                    Layout.topMargin: 40
                    Layout.bottomMargin: 6
                    label: "Colors"
                    implicitHeight: colorColumn.height
                    Layout.fillWidth: true
                    Column {
                        id: colorColumn
                        anchors.left: parent.left
                        anchors.right: parent.right
                        Text {
                            text: "Colors"
                            color: Theme.white
                            font.pixelSize: 14
                            font.weight: Font.DemiBold
                        }
                        Item {
                            anchors.left: parent.left
                            anchors.right: parent.right
                            height: 10
                        }
                        Rectangle {
                            color: Theme.darkGray2
                            implicitHeight: colorPane.implicitHeight + 20
                            anchors.left: parent.left
                            anchors.right: parent.right
                            GridLayout {
                                id: colorPane
                                anchors.fill: parent
                                anchors.topMargin: 10
                                anchors.bottomMargin: 10
                                anchors.leftMargin: 3
                                anchors.rightMargin: 3
                                rowSpacing: 8
                                columnSpacing: 20
                                columns: 2

                                readonly property var defaultPalette: Mixxx.Config.getHotcueColorPalette(hotcuePaletteComboBox.currentText)

                                onDefaultPaletteChanged: {
                                    hotcuePaletteColorIndex = 0
                                    loopPaletteColorIndex = 1
                                    jumpPaletteColorIndex = 2
                                }
                                property int hotcuePaletteColorIndex: 0
                                property int loopPaletteColorIndex: 0
                                property int jumpPaletteColorIndex: 0

                                RowLayout {
                                    Layout.preferredWidth: (colorPane.width - 56) * 0.5
                                    Layout.leftMargin: 14
                                    Layout.rightMargin: 14
                                    Mixxx.SettingParameter {
                                        label: "Track palette"
                                        Layout.fillWidth: true
                                        Text {
                                            anchors.fill: parent

                                            horizontalAlignment: Text.AlignLeft
                                            verticalAlignment: Text.AlignVCenter

                                            text: parent.label
                                            color: Theme.white
                                            font.pixelSize: 14
                                            font.weight: Font.Medium
                                        }
                                    }
                                    ColorPaletteComboBox {
                                        onCurrentIndexChanged: themeColorTab.dirty = true
                                        id: trackPaletteComboBox
                                    }
                                }
                                RowLayout {
                                    Layout.preferredWidth: (colorPane.width - 56) * 0.5
                                    Layout.leftMargin: 14
                                    Layout.rightMargin: 14
                                    Mixxx.SettingParameter {
                                        label: "Hotcue palette"
                                        Layout.fillWidth: true
                                        Text {
                                            anchors.fill: parent

                                            horizontalAlignment: Text.AlignLeft
                                            verticalAlignment: Text.AlignVCenter

                                            text: parent.label
                                            color: Theme.white
                                            font.pixelSize: 14
                                            font.weight: Font.Medium
                                        }
                                    }
                                    ColorPaletteComboBox {
                                        onCurrentIndexChanged: themeColorTab.dirty = true
                                        id: hotcuePaletteComboBox
                                    }
                                }
                                RowLayout {
                                    Layout.preferredWidth: (colorPane.width - 56) * 0.5
                                    Layout.leftMargin: 14
                                    Layout.rightMargin: 14
                                    Mixxx.SettingParameter {
                                        label: "Key color"
                                        Layout.fillWidth: true
                                        Text {
                                            anchors.fill: parent

                                            horizontalAlignment: Text.AlignLeft
                                            verticalAlignment: Text.AlignVCenter

                                            text: parent.label
                                            color: Theme.white
                                            font.pixelSize: 14
                                            font.weight: Font.Medium
                                        }
                                    }
                                    ColorPaletteComboBox {
                                        onCurrentIndexChanged: themeColorTab.dirty = true
                                        id: keyPaletteComboBox
                                        enabled: keyPaletteInput.enabled
                                        opacity: enabled ? 1 : 0.4
                                        model: Mixxx.Config.paletteNames().filter(palette => Mixxx.Config.colorPalette(palette).length == 12)
                                    }
                                    RatioChoice {
                                        onSelectedChanged: themeColorTab.dirty = true
                                        id: keyPaletteInput
                                        readonly property bool enabled: selected == "on"
                                        options: [
                                                  "on",
                                                  "off"
                                        ]
                                    }
                                }
                                Rectangle {
                                    Layout.preferredWidth: (colorPane.width - 56) * 0.5
                                    Layout.rightMargin: 14
                                    color: Theme.darkGray3
                                    implicitHeight: 45
                                    RowLayout {
                                        id: defaultCueColorsRow
                                        anchors.fill: parent
                                        anchors.topMargin: 5
                                        anchors.leftMargin: 5
                                        anchors.rightMargin: 10
                                        ColumnLayout {
                                            Layout.fillHeight: true
                                            spacing: 15
                                            Mixxx.SettingParameter {
                                                label: "Hotcue default color palette"
                                                Layout.fillWidth: true
                                                Text {
                                                    anchors.fill: parent

                                                    horizontalAlignment: Text.AlignHCenter
                                                    verticalAlignment: Text.AlignVCenter

                                                    text: parent.label
                                                    color: Theme.white
                                                    font.pixelSize: 14
                                                    font.weight: Font.Medium
                                                }
                                            }
                                            DefaultColorSelector {
                                                onCurrentIndexChanged: themeColorTab.dirty = true
                                                id: hotcuePaletteInput
                                                Layout.alignment: Qt.AlignHCenter
                                                Layout.fillWidth: true
                                                palette: colorPane.defaultPalette
                                                currentIndex: colorPane.hotcuePaletteColorIndex
                                            }
                                        }
                                        ColumnLayout {
                                            Layout.fillHeight: true
                                            spacing: 15
                                            Mixxx.SettingParameter {
                                                label: "Loop default color palette"
                                                Layout.fillWidth: true
                                                Text {
                                                    anchors.fill: parent

                                                    horizontalAlignment: Text.AlignHCenter
                                                    verticalAlignment: Text.AlignVCenter

                                                    text: parent.label
                                                    color: Theme.white
                                                    font.pixelSize: 14
                                                    font.weight: Font.Medium
                                                }
                                            }
                                            DefaultColorSelector {
                                                onCurrentIndexChanged: themeColorTab.dirty = true
                                                id: loopPaletteInput
                                                Layout.alignment: Qt.AlignHCenter
                                                Layout.fillWidth: true
                                                palette: colorPane.defaultPalette
                                                currentIndex: colorPane.loopPaletteColorIndex
                                            }
                                        }
                                        ColumnLayout {
                                            id: jumpDefaultColor
                                            Layout.fillHeight: true
                                            spacing: 15
                                            Mixxx.SettingParameter {
                                                label: "Jump default color palette"
                                                Layout.fillWidth: true
                                                Text {
                                                    anchors.fill: parent

                                                    horizontalAlignment: Text.AlignHCenter
                                                    verticalAlignment: Text.AlignVCenter

                                                    text: parent.label
                                                    color: Theme.white
                                                    font.pixelSize: 14
                                                    font.weight: Font.Medium
                                                }
                                            }

                                            DefaultColorSelector {
                                                onCurrentIndexChanged: themeColorTab.dirty = true
                                                id: jumpPaletteInput
                                                Layout.fillWidth: true
                                                Layout.alignment: Qt.AlignHCenter
                                                palette: colorPane.defaultPalette
                                                currentIndex: colorPane.jumpPaletteColorIndex
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                Item {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }
            }
        }
        Mixxx.SettingGroup {
            property bool dirty: false
            id: waveformTab
            label: "Waveform"
            visible: root.selectedIndex == 1
            anchors.fill: parent

            onActivated: {
                root.selectedIndex = 1;
            }

            readonly property real playMarkerPosition: playMarkerPositionInput.value / 100
            readonly property real defaultZoom: defaultZoomInput.value / 10
            readonly property real beatgridOpacity: beatGridAlphaInput.value / 100
            property alias untilMarkShowTime: untilMarkShowTimeInput.enabled
            property alias untilMarkShowBeats: untilMarkShowBeatsInput.enabled
            readonly property var playMarkerAlign: untilMarkAlignInput.selected == "center" ? Qt.AlignHCenter : untilMarkAlignInput.selected == "top" ? Qt.AlignTop : Qt.AlignBottom
            readonly property double playMarkerTextSize: untilMarkTextPointSizeInput.value

            readonly property double globalGain: visualGainAllInput.value / 100
            readonly property double lowGain: visualGainLowInput.value / 100
            readonly property double middleGain: visualGainMediumInput.value / 100
            readonly property double highGain: visualGainHighInput.value / 100

            ColumnLayout {
                anchors.fill: parent
                anchors.topMargin: 20
                RowLayout {
                    Layout.leftMargin: 14
                    Layout.rightMargin: 14
                    ColumnLayout {
                        Layout.preferredWidth: root.width * 0.5
                        spacing: 15
                        GridLayout {
                            columns: 2
                            rowSpacing: 10
                            Mixxx.SettingParameter {
                                Layout.fillWidth: true
                                label: "End of track warning"
                                height: 14
                                Text {
                                    text: parent.label
                                    color: Theme.white
                                    font.pixelSize: 14
                                }
                            }
                            SettingComponents.Slider {
                                onValueChanged: waveformTab.dirty = true
                                id: endOfTrackWarningInput
                                Layout.preferredWidth: waveformTab.width * 0.35
                                markers: [0, 10, 30, 60, 120]
                                suffix: "sec"
                                value: 30
                                max: 120
                            }
                            Mixxx.SettingParameter {
                                Layout.fillWidth: true
                                label: "Beat grid opacity"
                                height: 14
                                Text {
                                    text: parent.label
                                    color: Theme.white
                                    font.pixelSize: 14
                                }
                            }
                            SettingComponents.Slider {
                                onValueChanged: waveformTab.dirty = true
                                id: beatGridAlphaInput
                                Layout.preferredWidth: waveformTab.width * 0.35
                                markers: [0, 50, 100]
                                suffix: "%"
                                value: 50
                            }
                            Mixxx.SettingParameter {
                                Layout.fillWidth: true
                                label: "Default zoom level"
                                height: 14
                                Text {
                                    text: parent.label
                                    color: Theme.white
                                    font.pixelSize: 14
                                }
                            }
                            SettingComponents.Slider {
                                onValueChanged: waveformTab.dirty = true
                                id: defaultZoomInput
                                Layout.preferredWidth: waveformTab.width * 0.35
                                markers: [0, 50, 100]
                                suffix: "%"
                                value: 60
                            }
                        }
                        ColumnLayout {
                            RowLayout {
                                Mixxx.SettingParameter {
                                    Layout.fillWidth: true
                                    label: "Synchronise zoom level across waveform"
                                    height: 14
                                    Text {
                                        text: parent.label
                                        color: Theme.white
                                        font.pixelSize: 14
                                    }
                                }
                                RatioChoice {
                                    onSelectedChanged: waveformTab.dirty = true
                                    id: synchronizeAllZoomLevelInput
                                    options: [
                                              "on",
                                              "off"
                                    ]
                                }
                            }
                            RowLayout {
                                Mixxx.SettingParameter {
                                    Layout.fillWidth: true
                                    label: "Normalise waveform overview"
                                    height: 14
                                    Text {
                                        text: parent.label
                                        color: Theme.white
                                        font.pixelSize: 14
                                    }
                                }
                                RatioChoice {
                                    onSelectedChanged: waveformTab.dirty = true
                                    id: normaliseWaveformOverviewLevelInput
                                    options: [
                                              "on",
                                              "off"
                                    ]
                                }
                            }
                        }
                    }
                    ColumnLayout {
                        Layout.preferredWidth: root.width * 0.5
                        ComboBox {
                            onCurrentIndexChanged: waveformTab.dirty = true
                            id: waveformDropdown

                            Layout.fillWidth: true
                            Layout.preferredHeight: 102

                            readonly property list<var> indices: [
                                Mixxx.WaveformDisplay.Filtered,
                                Mixxx.WaveformDisplay.HSV,
                                Mixxx.WaveformDisplay.RGB,
                                Mixxx.WaveformDisplay.Simple,
                                Mixxx.WaveformDisplay.Stacked,
                            ]
                            readonly property var currentType: indices[currentIndex]
                            property int options: Mixxx.WaveformDisplay.Options.None

                            model: ["Filtered", "HSV", "RGB", "Simple", "Stacked"]

                            readonly property var currentSignalRender: (currentIndex == 0 ? filteredRenderer :
                                currentIndex == 1 ? hsvRenderer :
                                currentIndex == 2 ? rgbRenderer :
                                currentIndex == 3 ? simpleRenderer :
                                    stackedRenderer)

                            property var track: null
                            property double playbackProgress: 0.2

                            property var backgroundColor: "#0F0F0E"
                            property var axesColor: '#a1a1a1a1'
                            property var lowColor: '#2154D7'
                            property var midColor: '#97632D'
                            property var highColor: '#D5C2A2'

                            property var filteredRenderer: Mixxx.WaveformRendererFiltered {
                                axesColor: waveformDropdown.axesColor
                                lowColor: waveformDropdown.lowColor
                                midColor: waveformDropdown.midColor
                                highColor: waveformDropdown.highColor

                                gainAll: waveformTab.globalGain
                                gainLow: waveformTab.lowGain
                                gainMid: waveformTab.middleGain
                                gainHigh: waveformTab.highGain

                                ignoreStem: true
                            }
                            property var hsvRenderer: Mixxx.WaveformRendererHSV {
                                axesColor: waveformDropdown.axesColor
                                color: waveformDropdown.lowColor

                                gainAll: waveformTab.globalGain
                                gainLow: waveformTab.lowGain
                                gainMid: waveformTab.middleGain
                                gainHigh: waveformTab.highGain

                                ignoreStem: true
                            }
                            property var rgbRenderer: Mixxx.WaveformRendererRGB {
                                axesColor: waveformDropdown.axesColor
                                lowColor: waveformDropdown.lowColor
                                midColor: waveformDropdown.midColor
                                highColor: waveformDropdown.highColor

                                gainAll: waveformTab.globalGain
                                gainLow: waveformTab.lowGain
                                gainMid: waveformTab.middleGain
                                gainHigh: waveformTab.highGain

                                ignoreStem: true
                            }
                            property var simpleRenderer: Mixxx.WaveformRendererSimple {
                                axesColor: waveformDropdown.axesColor
                                color: waveformDropdown.lowColor
                                gain: waveformTab.globalGain
                                ignoreStem: true
                            }
                            property var stackedRenderer: Mixxx.WaveformRendererFiltered {
                                stacked: true
                                axesColor: waveformDropdown.axesColor
                                lowColor: waveformDropdown.lowColor
                                midColor: waveformDropdown.midColor
                                highColor: waveformDropdown.highColor

                                gainAll: waveformTab.globalGain
                                gainLow: waveformTab.lowGain
                                gainMid: waveformTab.middleGain
                                gainHigh: waveformTab.highGain

                                ignoreStem: true
                            }
                            property var beatsRenderer: Mixxx.WaveformRendererBeat {
                                color: Qt.alpha('#a1a1a1', waveformTab.beatgridOpacity)
                            }

                            readonly property var player: Mixxx.PlayerManager.getPlayer("[Channel1]")

                            Timer {
                                interval: 25; running: !waveformDropdown.player?.isLoaded && waveformDropdown.track !== null; repeat : true
                                onTriggered: {
                                    waveformDropdown.playbackProgress += interval / 1000 / waveformDropdown.track.duration
                                    if (waveformDropdown.playbackProgress > 0.8) {
                                        waveformDropdown.playbackProgress = 0.2
                                    }
                                }
                            }

                            delegate: ItemDelegate {
                                id: control

                                required property int index
                                width: parent.width
                                height: 99
                                highlighted: waveformDropdown.highlightedIndex === this.index

                                ColumnLayout {
                                    anchors.fill: parent
                                    Item {
                                        Layout.fillHeight: true
                                        Layout.fillWidth: true

                                        Mixxx.WaveformDisplay {
                                            id: waveform
                                            visible: false
                                            anchors.fill: parent
                                            zoom: waveformTab.defaultZoom
                                            backgroundColor: "#0F0F0E"

                                            player: waveformDropdown.player.isLoaded ? waveformDropdown.player : null
                                            track: waveformDropdown.player.isLoaded ? null : waveformDropdown.track
                                            position: waveformDropdown.playbackProgress

                                            renderers: [
                                                        (index == 0 ? waveformDropdown.filteredRenderer :
                                                        index == 1 ? waveformDropdown.hsvRenderer :
                                                        index == 2 ? waveformDropdown.rgbRenderer :
                                                        index == 3 ? waveformDropdown.simpleRenderer :
                                                            waveformDropdown.stackedRenderer),
                                                        waveformDropdown.beatsRenderer
                                            ]
                                            options: (waveformOptionHighDetailsInput.enabled ? Mixxx.WaveformDisplay.Options.HighDetail : 0) + (waveformOptionStereoInput.enabled ? Mixxx.WaveformDisplay.Options.SplitStereoSignal : 0)
                                        }
                                        InnerShadow {
                                            id: effect1
                                            anchors.fill: parent
                                            source: waveform
                                            spread: 0.2
                                            radius: 24
                                            samples: 24
                                            horizontalOffset: 4
                                            verticalOffset: 4
                                            color: control.highlighted ? "#575757" : "#000000"
                                        }
                                        InnerShadow {
                                            anchors.fill: parent
                                            source: effect1
                                            spread: 0.2
                                            radius: 24
                                            samples: 24
                                            horizontalOffset: -4
                                            verticalOffset: -4
                                            color: control.highlighted ? "#575757" : "#000000"
                                        }
                                    }
                                    Text {
                                        Layout.bottomMargin: 5
                                        Layout.leftMargin: 5
                                        text: waveformDropdown.model[index]
                                        color: "#FFFFFF"
                                    }
                                }

                                background: Rectangle {
                                    radius: 5
                                    color: control.pressed || control.highlighted ? "#575757" : "#000000"
                                }
                            }

                            onActivated: (selectedIndex) => {
                                currentIndex = selectedIndex
                            }

                            contentItem: Rectangle {
                                color: '#00000000'
                                ColumnLayout {
                                    anchors.fill: parent
                                    Item {
                                        Layout.fillHeight: true
                                        Layout.fillWidth: true
                                        property var markRenderer: Mixxx.WaveformRendererMark {
                                            playMarkerColor: '#D9D9D9'
                                            playMarkerBackground: '#D9D9D9'
                                            playMarkerPosition: waveformTab.playMarkerPosition
                                            defaultMark: Mixxx.WaveformMark {
                                                align: "bottom|right"
                                                color: "#00d9ff"
                                                textColor: "#1a1a1a"
                                                text: " %1 "
                                            }

                                            untilMark.showTime: waveformTab.untilMarkShowTime
                                            untilMark.showBeats: waveformTab.untilMarkShowBeats
                                            untilMark.align: waveformTab.playMarkerAlign
                                            untilMark.textSize: waveformTab.playMarkerTextSize
                                            untilMark.defaultNextMarkPosition: waveformDropdown.player.isLoaded || waveformDropdown.track == null ? -1 : waveformDropdown.track.duration * waveformDropdown.track.sampleRate * 2
                                        }
                                        Mixxx.WaveformDisplay {
                                            id: waveform
                                            visible: false
                                            anchors.fill: parent
                                            zoom: waveformTab.defaultZoom
                                            backgroundColor: "#0F0F0E"

                                            player: waveformDropdown.player.isLoaded ? waveformDropdown.player : null
                                            track: waveformDropdown.player.isLoaded ? null : waveformDropdown.track
                                            position: waveformDropdown.playbackProgress

                                            options: (waveformOptionHighDetailsInput.enabled ? Mixxx.WaveformDisplay.Options.HighDetail : 0) + (waveformOptionStereoInput.enabled ? Mixxx.WaveformDisplay.Options.SplitStereoSignal : 0)

                                            renderers: [
                                                        waveformDropdown.currentSignalRender,
                                                        waveformDropdown.beatsRenderer,
                                                        parent.markRenderer
                                            ]
                                        }
                                        InnerShadow {
                                            id: effect1
                                            anchors.fill: parent
                                            source: waveform
                                            spread: 0.2
                                            radius: 24
                                            samples: 24
                                            horizontalOffset: 2
                                            verticalOffset: 4
                                            color: "#000000"
                                        }
                                        InnerShadow {
                                            anchors.fill: parent
                                            source: effect1
                                            spread: 0.2
                                            radius: 24
                                            samples: 24
                                            horizontalOffset: -2
                                            verticalOffset: -4
                                            color: "#000000"
                                        }
                                    }
                                    RowLayout {
                                        Layout.fillWidth: true
                                        Text {
                                            Layout.fillWidth: true
                                            Layout.margins: 5
                                            text: waveformDropdown.currentText
                                            color: "#FFFFFF"
                                            font.weight: Font.DemiBold
                                        }
                                        Text {
                                            visible: waveformDropdown.currentSignalRender.supportedOptions & Mixxx.WaveformDisplay.Options.SplitStereoSignal
                                            Layout.margins: 5
                                            text: "Stereo split"
                                            font.pixelSize: 11
                                            color: "#FFFFFF"
                                        }
                                        RatioChoice {
                                            visible: waveformDropdown.currentSignalRender.supportedOptions & Mixxx.WaveformDisplay.Options.SplitStereoSignal
                                            onSelectedChanged: waveformTab.dirty = true
                                            id: waveformOptionStereoInput
                                            inactiveColor: Theme.darkGray4
                                            metric.font.pixelSize: 11
                                            content.height: 18

                                            readonly property bool enabled: selected == "on"
                                            options: [
                                                      "on",
                                                      "off"
                                            ]
                                        }
                                        Text {
                                            visible: waveformDropdown.currentSignalRender.supportedOptions & Mixxx.WaveformDisplay.Options.HighDetail
                                            Layout.leftMargin: 10
                                            Layout.margins: 5
                                            text: "High details"
                                            font.pixelSize: 11
                                            color: "#FFFFFF"
                                        }
                                        RatioChoice {
                                            visible: waveformDropdown.currentSignalRender.supportedOptions & Mixxx.WaveformDisplay.Options.HighDetail
                                            onSelectedChanged: waveformTab.dirty = true
                                            id: waveformOptionHighDetailsInput
                                            inactiveColor: Theme.darkGray4
                                            metric.font.pixelSize: 11
                                            content.height: 18

                                            readonly property bool enabled: selected == "on"

                                            options: [
                                                      "on",
                                                      "off"
                                            ]
                                        }
                                        Text {
                                            Layout.leftMargin: 50
                                            Layout.margins: 5
                                            text: "Color"
                                            font.pixelSize: 11
                                            color: "#FFFFFF"
                                        }
                                        Repeater {
                                            model: waveformDropdown.currentIndex != 3 ? [waveformDropdown.lowColor, waveformDropdown.midColor, waveformDropdown.highColor] : [waveformDropdown.lowColor]
                                            Rectangle {
                                                required property color modelData
                                                required property int index

                                                Layout.margins: 1
                                                Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
                                                width: 15
                                                height: 15
                                                radius: 2
                                                color: modelData

                                                ColorDialog {
                                                    id: colorDialog
                                                    title: "Please choose a color"
                                                    options: ColorDialog.ShowAlphaChannel
                                                    onAccepted: {
                                                        if (index == 0) {
                                                            waveformDropdown.lowColor = Qt.color(selectedColor);
                                                        } else if (index == 1) {
                                                            waveformDropdown.midColor = Qt.color(selectedColor);
                                                        } else {
                                                            waveformDropdown.highColor = Qt.color(selectedColor);
                                                        }
                                                    }
                                                    selectedColor: modelData
                                                }
                                                MouseArea {
                                                    anchors.fill: parent
                                                    cursorShape: Qt.PointingHandCursor
                                                    onPressed: {
                                                        console.log("PICKING",colorDialog, colorDialog.visible)
                                                        colorDialog.open()
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                                // leftPadding: 5
                                // rightPadding: waveformDropdown.indicator.width + waveformDropdown.spacing
                            }

                            background: Rectangle {
                                radius: 5
                                // border.width: control.highlighted ?  1 : 0
                                // border.color: Theme.deckLineColor
                                color: "#000000"
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
                                x: waveformDropdown.x + 27
                                y: waveformDropdown.height + 13
                                width: waveformDropdown.width - 27
                                implicitHeight: contentItem.implicitHeight

                                contentItem: ListView {
                                    clip: true
                                    anchors.fill: parent
                                    implicitHeight: contentHeight
                                    model: waveformDropdown.popup.visible ? waveformDropdown.delegateModel : null
                                    currentIndex: waveformDropdown.highlightedIndex

                                    ScrollIndicator.vertical: ScrollIndicator {
                                    }
                                }

                                background: Rectangle {
                                    border.width: 0
                                    radius: 8
                                    color: '#000000'
                                }
                            }
                        }
                        SettingComponents.Slider {
                            onValueChanged: waveformTab.dirty = true
                            id: playMarkerPositionInput
                            Layout.fillWidth: true
                            Layout.topMargin: 15
                            Layout.preferredHeight: 23

                            markers: [0, 50, 100]
                            suffix: "%"
                            value: 50
                        }
                        Text {
                            Layout.topMargin: 10
                            Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
                            text: "Play marker position"
                            color: "#D9D9D9"
                            font.pixelSize: 14
                        }
                    }
                }

                Mixxx.SettingGroup {
                    Layout.topMargin: 40
                    Layout.bottomMargin: 6
                    label: "Play marker hints"
                    implicitHeight: playMarkerColumn.height
                    Layout.fillWidth: true
                    Column {
                        id: playMarkerColumn
                        anchors.left: parent.left
                        anchors.right: parent.right
                        Text {
                            text: "Play marker hints"
                            color: Theme.white
                            font.pixelSize: 14
                            font.weight: Font.DemiBold
                        }
                        Item {
                            anchors.left: parent.left
                            anchors.right: parent.right
                            height: 10
                        }
                        Rectangle {
                            color: Theme.darkGray2
                            implicitHeight: playMarkerHintsPane.implicitHeight + 20
                            anchors.left: parent.left
                            anchors.right: parent.right
                            GridLayout {
                                id: playMarkerHintsPane
                                anchors.fill: parent
                                anchors.topMargin: 10
                                anchors.bottomMargin: 10
                                anchors.leftMargin: 17
                                anchors.rightMargin: 17
                                rowSpacing: 8
                                columnSpacing: 20
                                columns: 2

                                RowLayout {
                                    Layout.preferredWidth: (playMarkerHintsPane.width - 34) * 0.5
                                    Mixxx.SettingParameter {
                                        label: "Beats until next marker"
                                        Layout.fillWidth: true
                                        Text {
                                            anchors.fill: parent

                                            horizontalAlignment: Text.AlignLeft
                                            verticalAlignment: Text.AlignVCenter

                                            text: parent.label
                                            color: Theme.white
                                            font.pixelSize: 14
                                            font.weight: Font.Medium
                                        }
                                    }
                                    RatioChoice {
                                        onSelectedChanged: waveformTab.dirty = true
                                        id: untilMarkShowBeatsInput
                                        inactiveColor: Theme.darkGray4
                                        readonly property bool enabled: selected == "on"
                                        options: [
                                                  "on",
                                                  "off"
                                        ]
                                        function load(value) {
                                            untilMarkShowBeatsInput.selected = value ? "on" : "off"
                                        }
                                    }
                                }

                                RowLayout {
                                    Layout.preferredWidth: (playMarkerHintsPane.width - 34) * 0.5
                                    Mixxx.SettingParameter {
                                        label: "Placement"
                                        Layout.fillWidth: true
                                        Text {
                                            anchors.fill: parent

                                            horizontalAlignment: Text.AlignLeft
                                            verticalAlignment: Text.AlignVCenter

                                            text: parent.label
                                            color: Theme.white
                                            font.pixelSize: 14
                                            font.weight: Font.Medium
                                        }
                                    }
                                    RatioChoice {
                                        onSelectedChanged: waveformTab.dirty = true
                                        id: untilMarkAlignInput
                                        options: [
                                                  "top",
                                                  "center",
                                                  "bottom"
                                        ]
                                        selected: "center"

                                        function load(value) {
                                            switch (value) {
                                                case 1:
                                                    untilMarkAlignInput.selected = "top"
                                                    break;
                                                case 3:
                                                    untilMarkAlignInput.selected = "bottom"
                                                    break;
                                                default:
                                                    console.warn(`unrecognised value '${value}' for Waveform,UntilMarkAlign. Defaulting to center (2).`)
                                                case 2:
                                                        untilMarkAlignInput.selected = "center"
                                                    break;
                                            }
                                        }
                                    }
                                }

                                RowLayout {
                                    Layout.preferredWidth: (playMarkerHintsPane.width - 34) * 0.5
                                    Mixxx.SettingParameter {
                                        label: "Time until next marker"
                                        Layout.fillWidth: true
                                        Text {
                                            anchors.fill: parent

                                            horizontalAlignment: Text.AlignLeft
                                            verticalAlignment: Text.AlignVCenter

                                            text: parent.label
                                            color: Theme.white
                                            font.pixelSize: 14
                                            font.weight: Font.Medium
                                        }
                                    }
                                    RatioChoice {
                                        onSelectedChanged: waveformTab.dirty = true
                                        id: untilMarkShowTimeInput
                                        inactiveColor: Theme.darkGray4
                                        readonly property bool enabled: selected == "on"
                                        options: [
                                                  "on",
                                                  "off"
                                        ]
                                        function load(value) {
                                            untilMarkShowTimeInput.selected = value ? "on" : "off"
                                        }
                                    }
                                }
                                RowLayout {
                                    Layout.preferredWidth: (playMarkerHintsPane.width - 34) * 0.5
                                    Mixxx.SettingParameter {
                                        label: "Font size"
                                        Layout.fillWidth: true
                                        Text {
                                            anchors.fill: parent

                                            horizontalAlignment: Text.AlignLeft
                                            verticalAlignment: Text.AlignVCenter

                                            text: parent.label
                                            color: Theme.white
                                            font.pixelSize: 14
                                            font.weight: Font.Medium
                                        }
                                    }
                                    Skin.ComboBox {
                                        readonly property list<int> fontSizes: [
                                            10,
                                            12,
                                            15,
                                            18,
                                            24,
                                            32,
                                            48
                                        ]
                                        readonly property int value: fontSizes[currentIndex] ?? 10
                                        onCurrentIndexChanged: waveformTab.dirty = true
                                        id: untilMarkTextPointSizeInput
                                        model: fontSizes.map(i => `${i} pt`)
                                    }
                                }
                            }
                        }
                    }
                }

                Mixxx.SettingGroup {
                    Layout.topMargin: 40
                    Layout.bottomMargin: 6
                    label: "Visual gain"
                    implicitHeight: visualGainColumn.height
                    Layout.fillWidth: true
                    Column {
                        id: visualGainColumn
                        anchors.left: parent.left
                        anchors.right: parent.right
                        RowLayout {
                            anchors.left: parent.left
                            anchors.right: parent.right
                            Text {
                                Layout.fillWidth: true
                                text: "Visual gain"
                                color: Theme.white
                                font.pixelSize: 14
                                font.weight: Font.DemiBold
                            }
                            Text {
                                horizontalAlignment: Text.AlignLeft
                                verticalAlignment: Text.AlignVCenter

                                text: "Global"
                                color: Theme.white
                                font.pixelSize: 14
                                font.weight: Font.Medium
                            }
                            SettingComponents.Slider {
                                onValueChanged: waveformTab.dirty = true
                                id: visualGainAllInput
                                width: 400
                                value: 100
                                markers: [0, 50, 100, 150]
                                suffix: "%"
                                max: 200
                            }
                        }
                        Item {
                            anchors.left: parent.left
                            anchors.right: parent.right
                            height: 10
                        }
                        Rectangle {
                            color: Theme.darkGray2
                            implicitHeight: visualGainPane.implicitHeight + 56
                            anchors.left: parent.left
                            anchors.right: parent.right

                            RowLayout {
                                id: visualGainPane
                                anchors.fill: parent
                                anchors.topMargin: 28
                                anchors.bottomMargin: 28
                                anchors.leftMargin: 17
                                anchors.rightMargin: 17
                                spacing: 50
                                ColumnLayout {
                                    Layout.preferredWidth: visualGainPane.width * 0.33
                                    Mixxx.SettingParameter {
                                        label: "Low"
                                        Layout.fillWidth: true
                                        Text {
                                            anchors.fill: parent

                                            horizontalAlignment: Text.AlignVCenter
                                            verticalAlignment: Text.AlignVCenter

                                            text: parent.label
                                            color: Theme.white
                                            font.pixelSize: 14
                                            font.weight: Font.Medium
                                        }
                                    }
                                    SettingComponents.Slider {
                                        onValueChanged: waveformTab.dirty = true
                                        Layout.fillWidth: true
                                        id: visualGainLowInput
                                        value: 100
                                        markers: [0, 50, 100, 150]
                                        suffix: "%"
                                        max: 200
                                    }
                                }
                                ColumnLayout {
                                    Layout.preferredWidth: visualGainPane.width * 0.33
                                    Mixxx.SettingParameter {
                                        label: "Middle"
                                        Layout.fillWidth: true
                                        Text {
                                            anchors.fill: parent

                                            horizontalAlignment: Text.AlignHCenter
                                            verticalAlignment: Text.AlignVCenter

                                            text: parent.label
                                            color: Theme.white
                                            font.pixelSize: 14
                                            font.weight: Font.Medium
                                        }
                                    }
                                    SettingComponents.Slider {
                                        onValueChanged: waveformTab.dirty = true
                                        Layout.fillWidth: true
                                        id: visualGainMediumInput
                                        value: 100
                                        markers: [0, 50, 100, 150]
                                        suffix: "%"
                                        max: 200
                                    }
                                }
                                ColumnLayout {
                                    Layout.preferredWidth: visualGainPane.width * 0.33
                                    Mixxx.SettingParameter {
                                        label: "High"
                                        Layout.fillWidth: true
                                        Text {
                                            anchors.fill: parent

                                            horizontalAlignment: Text.AlignHCenter
                                            verticalAlignment: Text.AlignVCenter

                                            text: parent.label
                                            color: Theme.white
                                            font.pixelSize: 14
                                            font.weight: Font.Medium
                                        }
                                    }
                                    SettingComponents.Slider {
                                        onValueChanged: waveformTab.dirty = true
                                        Layout.fillWidth: true
                                        id: visualGainHighInput
                                        value: 100
                                        markers: [0, 50, 100, 150]
                                        suffix: "%"
                                        max: 200
                                    }
                                }
                            }
                        }
                    }
                }

                Item {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }
            }
        }
        Mixxx.SettingGroup {
            id: decksTab
            label: "Decks"
            visible: root.selectedIndex == 2
            anchors.fill: parent

            property bool dirty: false

            onActivated: {
                root.selectedIndex = 2;
            }
            ColumnLayout {
                anchors.fill: parent
                anchors.topMargin: 20
                spacing: 0
                GridLayout {
                    id: deckPane
                    // anchors.fill: parent
                    // anchors.bottomMargin: 10
                    // anchors.leftMargin: 3
                    // anchors.rightMargin: 3
                    rowSpacing: 15
                    columnSpacing: 20
                    columns: 2
                    RowLayout {
                        Layout.preferredWidth: (deckPane.width - 56) * 0.5
                        Layout.leftMargin: 14
                        Layout.rightMargin: 14
                        Mixxx.SettingParameter {
                            label: "Cue mode"
                            Layout.fillWidth: true
                            Text {
                                anchors.fill: parent

                                horizontalAlignment: Text.AlignLeft
                                verticalAlignment: Text.AlignVCenter

                                text: parent.label
                                color: Theme.white
                                font.pixelSize: 14
                                font.weight: Font.Medium
                            }
                        }
                        Skin.ComboBox {
                            onCurrentIndexChanged: decksTab.dirty = true
                            id: cueModeInput
                            implicitWidth: 200
                            model: [
                                    "Mixxx",
                                    "Mixxx (no blinking)",
                                    "Pioneer",
                                    "Denon",
                                    "Numark",
                                    "CUP",
                            ]
                        }
                    }
                    RowLayout {
                        Layout.preferredWidth: (deckPane.width - 56) * 0.5
                        Layout.rightMargin: 14
                        Mixxx.SettingParameter {
                            label: "Time format"
                            Layout.fillWidth: true
                            Text {
                                anchors.fill: parent

                                horizontalAlignment: Text.AlignLeft
                                verticalAlignment: Text.AlignVCenter

                                text: parent.label
                                color: Theme.white
                                font.pixelSize: 14
                                font.weight: Font.Medium
                            }
                        }
                        Skin.ComboBox {
                            onCurrentIndexChanged: decksTab.dirty = true
                            id: timeMode
                            Layout.preferredWidth: (deckPane.width - 56) * 0.2
                            model: [
                                    DeckComponents.TrackTime.Mode.Traditional,
                                    DeckComponents.TrackTime.Mode.TraditionalCoarse,
                                    DeckComponents.TrackTime.Mode.Seconds,
                                    DeckComponents.TrackTime.Mode.SecondsLong,
                                    DeckComponents.TrackTime.Mode.KiloSeconds,
                                    DeckComponents.TrackTime.Mode.HectoSeconds,
                            ]
                            component Content: RowLayout {
                                required property var mode
                                Text {
                                    Layout.fillWidth: true
                                    text: {
                                        switch (parent.mode) {
                                            case DeckComponents.TrackTime.Mode.TraditionalCoarse:
                                                return "Traditional (Coarse)"
                                            case DeckComponents.TrackTime.Mode.Seconds:
                                                    return "Seconds"
                                            case DeckComponents.TrackTime.Mode.SecondsLong:
                                                return "Seconds (Long)"
                                            case DeckComponents.TrackTime.Mode.KiloSeconds:
                                                    return "Kiloseconds"
                                            case DeckComponents.TrackTime.Mode.HectoSeconds:
                                                return "Hectoseconds"
                                            default:
                                                console.warn(`Unsupported track time mode: ${root.mode}. Defaulting to traditional`)
                                            case DeckComponents.TrackTime.Mode.Traditional:
                                                    return "Traditional"
                                        }
                                    }
                                    color: Theme.deckTextColor
                                    elide: Text.ElideRight
                                    verticalAlignment: Text.AlignVCenter
                                }
                                DeckComponents.TrackTime {
                                    Layout.rightMargin: 17
                                    mode: parent.mode
                                    display: trackTimeDisplayInput.selected === "elapsed" ? DeckComponents.TrackTime.Display.Elapsed : trackTimeDisplayInput.selected === "remaining" ? DeckComponents.TrackTime.Display.Remaining : DeckComponents.TrackTime.Display.Both
                                    elapsed: 83.45
                                    remaining: 147.23
                                }
                            }

                            delegate: ItemDelegate {
                                id: itemDlgt

                                required property int index

                                width: parent.width
                                highlighted: root.highlightedIndex === this.index
                                text: content.text

                                contentItem: Content {
                                    id: content
                                    mode: timeMode.model[index]
                                    anchors.leftMargin: 10
                                    anchors.rightMargin: 20
                                    anchors.fill: parent
                                }

                                background: Rectangle {
                                    radius: 5
                                    border.width: itemDlgt.highlighted ? 1 : 0
                                    border.color: Theme.deckLineColor
                                    color: "transparent"
                                }
                            }

                            contentItem: Content {
                                mode: timeMode.model[timeMode.currentIndex]
                                anchors.leftMargin: 10
                                anchors.rightMargin: 20
                                anchors.fill: parent
                            }
                        }
                    }
                    RowLayout {
                        Layout.preferredWidth: (deckPane.width - 56) * 0.5
                        Layout.leftMargin: 14
                        Layout.rightMargin: 14
                        Mixxx.SettingParameter {
                            label: "Set intro start to main cue when analyzing tracks"
                            Layout.fillWidth: true
                            Text {
                                anchors.fill: parent

                                horizontalAlignment: Text.AlignLeft
                                verticalAlignment: Text.AlignVCenter

                                text: parent.label
                                color: Theme.white
                                font.pixelSize: 14
                                font.weight: Font.Medium
                            }
                        }
                        RatioChoice {
                            id: setIntroStartToMainCueInput
                            onSelectedChanged: decksTab.dirty = true
                            readonly property bool enabled: selected == "on"
                            options: [
                                      "on",
                                      "off"
                            ]
                        }
                    }
                    RowLayout {
                        Layout.preferredWidth: (deckPane.width - 56) * 0.5
                        Layout.rightMargin: 14
                        Mixxx.SettingParameter {
                            label: "Track time display"
                            Layout.fillWidth: true
                            Text {
                                anchors.fill: parent

                                horizontalAlignment: Text.AlignLeft
                                verticalAlignment: Text.AlignVCenter

                                text: parent.label
                                color: Theme.white
                                font.pixelSize: 14
                                font.weight: Font.Medium
                            }
                        }
                        RatioChoice {
                            onSelectedChanged: decksTab.dirty = true
                            id: trackTimeDisplayInput
                            options: [
                                      "elapsed",
                                      "remaining",
                                      "both"
                            ]
                        }
                    }
                    RowLayout {
                        Layout.preferredWidth: (deckPane.width - 56) * 0.5
                        Layout.leftMargin: 14
                        Layout.rightMargin: 14
                        Mixxx.SettingParameter {
                            label: "Double-press Load button to clone playing track"
                            Layout.fillWidth: true
                            Text {
                                anchors.fill: parent

                                horizontalAlignment: Text.AlignLeft
                                verticalAlignment: Text.AlignVCenter

                                text: parent.label
                                color: Theme.white
                                font.pixelSize: 14
                                font.weight: Font.Medium
                            }
                        }
                        RatioChoice {
                            id: doublePressLoadToCloneInput
                            onSelectedChanged: decksTab.dirty = true
                            options: [
                                      "on",
                                      "off"
                            ]
                        }
                    }
                    RowLayout {
                        Layout.preferredWidth: (deckPane.width - 56) * 0.5
                        Layout.rightMargin: 14
                        Mixxx.SettingParameter {
                            label: "Track load point"
                            Layout.fillWidth: true
                            Text {
                                anchors.fill: parent

                                horizontalAlignment: Text.AlignLeft
                                verticalAlignment: Text.AlignVCenter

                                text: parent.label
                                color: Theme.white
                                font.pixelSize: 14
                                font.weight: Font.Medium
                            }
                        }
                        Skin.ComboBox {
                            id: trackLoadPointInput
                            onCurrentIndexChanged: decksTab.dirty = true
                            model: [
                                    "Main cue",
                                    "Beginning of track",
                                    "First sound",
                                    "Intro start",
                                    "First hotcue",
                            ]
                        }
                    }
                    RowLayout {
                        Layout.preferredWidth: (deckPane.width - 56) * 0.5
                        Layout.leftMargin: 14
                        Layout.rightMargin: 14
                        Mixxx.SettingParameter {
                            label: "Loading a track when playing"
                            Layout.fillWidth: true
                            Text {
                                anchors.fill: parent

                                horizontalAlignment: Text.AlignLeft
                                verticalAlignment: Text.AlignVCenter

                                text: parent.label
                                color: Theme.white
                                font.pixelSize: 14
                                font.weight: Font.Medium
                            }
                        }
                        RatioChoice {
                            id: loadingTrackWhenPlayingInput
                            onSelectedChanged: decksTab.dirty = true
                            options: [
                                      "reject",
                                      "allow",
                                      "when stopped",
                            ]
                        }
                    }
                }
                Mixxx.SettingGroup {
                    Layout.topMargin: 40
                    Layout.bottomMargin: 6
                    label: "Speed & Key"
                    implicitHeight: speedKeyColumn.height
                    Layout.fillWidth: true
                    Column {
                        id: speedKeyColumn
                        anchors.left: parent.left
                        anchors.right: parent.right
                        Text {
                            anchors.leftMargin: 17
                            anchors.left: parent.left
                            text: "Speed & Key"
                            color: Theme.white
                            font.pixelSize: 14
                            font.weight: Font.DemiBold
                        }
                        Text {
                            anchors.leftMargin: 17
                            anchors.left: parent.left
                            text: "or tempo & pitch"
                            color: Theme.white
                            font.pixelSize: 11
                            font.weight: Font.Thin
                        }
                        Item {
                            anchors.left: parent.left
                            anchors.right: parent.right
                            height: 10
                        }
                        Rectangle {
                            color: Theme.darkGray2
                            implicitHeight: speedKeyPane.implicitHeight + 20
                            anchors.left: parent.left
                            anchors.right: parent.right
                            GridLayout {
                                id: speedKeyPane
                                anchors.fill: parent
                                anchors.topMargin: 10
                                anchors.bottomMargin: 10
                                anchors.leftMargin: 17
                                anchors.rightMargin: 17
                                rowSpacing: 15
                                columnSpacing: 20
                                columns: 2

                                RowLayout {
                                    Layout.preferredWidth: speedKeyPane.width * 0.5
                                    Mixxx.SettingParameter {
                                        Layout.fillWidth: true
                                        label: "Reset on track load"
                                        Text {
                                            anchors.fill: parent

                                            horizontalAlignment: Text.AlignLeft
                                            verticalAlignment: Text.AlignVCenter

                                            text: parent.label
                                            color: Theme.white
                                            font.pixelSize: 14
                                            font.weight: Font.Medium
                                        }
                                    }
                                    RatioChoice {
                                        id: resetOnTrackLoadInput
                                        onSelectedChanged: decksTab.dirty = true
                                        inactiveColor: Theme.darkGray4
                                        options: [
                                                  "none",
                                                  "key",
                                                  "both",
                                                  "tempo",
                                        ]
                                    }
                                }

                                RowLayout {
                                    Layout.preferredWidth: speedKeyPane.width * 0.5
                                    Mixxx.SettingParameter {
                                        Layout.fillWidth: true
                                        label: "Slider range"
                                        Text {
                                            anchors.fill: parent

                                            horizontalAlignment: Text.AlignLeft
                                            verticalAlignment: Text.AlignVCenter

                                            text: parent.label
                                            color: Theme.white
                                            font.pixelSize: 14
                                            font.weight: Font.Medium
                                        }
                                    }
                                    Skin.ComboBox {
                                        id: sliderRangeInput
                                        onCurrentIndexChanged: decksTab.dirty = true
                                        readonly property list<int> values: [
                                            4,
                                            6,
                                            8,
                                            10,
                                            16,
                                            24,
                                            50,
                                            90
                                        ]
                                        model: [
                                                "4%",
                                                "6% (semitone)",
                                                "8% (Technics SL-1210)",
                                                "10%",
                                                "16%",
                                                "24%",
                                                "50%",
                                                "90%"
                                        ]
                                    }
                                }

                                RowLayout {
                                    Layout.preferredWidth: speedKeyPane.width * 0.5
                                    Mixxx.SettingParameter {
                                        Layout.fillWidth: true
                                        label: "Sync mode"
                                        Text {
                                            anchors.fill: parent

                                            horizontalAlignment: Text.AlignLeft
                                            verticalAlignment: Text.AlignVCenter

                                            text: parent.label
                                            color: Theme.white
                                            font.pixelSize: 14
                                            font.weight: Font.Medium
                                        }
                                    }
                                    RatioChoice {
                                        id: syncModeInput
                                        onSelectedChanged: decksTab.dirty = true
                                        inactiveColor: Theme.darkGray4
                                        options: [
                                                  "follow soft leader",
                                                  "use steady"
                                        ]
                                    }
                                }

                                RowLayout {
                                    Layout.preferredWidth: speedKeyPane.width * 0.5
                                    Mixxx.SettingParameter {
                                        Layout.fillWidth: true
                                        label: "Slider orientation"
                                        height: 32
                                        Column {
                                            anchors.fill: parent
                                            Text {
                                                text: "Slider orientation"
                                                color: Theme.white
                                                font.pixelSize: 14
                                                font.weight: Font.Medium
                                            }
                                            Text {
                                                text: "Define which end of the slider will increase the pitch"
                                                color: Theme.white
                                                font.pixelSize: 11
                                                font.italic: true
                                                font.weight: Font.Thin
                                            }
                                        }
                                    }
                                    RatioChoice {
                                        id: sliderOrientationInput
                                        onSelectedChanged: decksTab.dirty = true
                                        inactiveColor: Theme.darkGray4
                                        options: [
                                                  "down",
                                                  "up"
                                        ]
                                    }
                                }

                                RowLayout {
                                    Layout.preferredWidth: speedKeyPane.width * 0.5
                                    Mixxx.SettingParameter {
                                        Layout.fillWidth: true
                                        label: "Keylock mode"
                                        Text {
                                            anchors.fill: parent

                                            horizontalAlignment: Text.AlignLeft
                                            verticalAlignment: Text.AlignVCenter

                                            text: parent.label
                                            color: Theme.white
                                            font.pixelSize: 14
                                            font.weight: Font.Medium
                                        }
                                    }
                                    RatioChoice {
                                        id: keylockModeInput
                                        onSelectedChanged: decksTab.dirty = true
                                        inactiveColor: Theme.darkGray4
                                        options: [
                                                  "original key",
                                                  "current key"
                                        ]
                                    }
                                }

                                RowLayout {
                                    Layout.preferredWidth: speedKeyPane.width * 0.5
                                    Mixxx.SettingParameter {
                                        Layout.fillWidth: true
                                        label: "Keyunlock mode"
                                        Text {
                                            anchors.fill: parent

                                            horizontalAlignment: Text.AlignLeft
                                            verticalAlignment: Text.AlignVCenter

                                            text: parent.label
                                            color: Theme.white
                                            font.pixelSize: 14
                                            font.weight: Font.Medium
                                        }
                                    }
                                    RatioChoice {
                                        id: keyunlockModeInput
                                        onSelectedChanged: decksTab.dirty = true
                                        inactiveColor: Theme.darkGray4
                                        options: [
                                                  "reset key",
                                                  "keep key"
                                        ]
                                    }
                                }

                                RowLayout {
                                    Layout.preferredWidth: speedKeyPane.width * 0.5
                                    Mixxx.SettingParameter {
                                        Layout.fillWidth: true
                                        label: "Pitch bend behaviour"
                                        Text {
                                            anchors.fill: parent

                                            horizontalAlignment: Text.AlignLeft
                                            verticalAlignment: Text.AlignVCenter

                                            text: parent.label
                                            color: Theme.white
                                            font.pixelSize: 14
                                            font.weight: Font.Medium
                                        }
                                    }
                                    RatioChoice {
                                        id: pitchBendBehaviourInput
                                        onSelectedChanged: decksTab.dirty = true
                                        inactiveColor: Theme.darkGray4
                                        options: [
                                                  "abrupt jump",
                                                  "smooth ramping",
                                        ]
                                    }
                                }

                                RowLayout {
                                    Layout.rowSpan: 3
                                    Layout.preferredWidth: speedKeyPane.width * 0.5
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
                                            text: "Adjustment buttons"
                                            color: Theme.white
                                            font.pixelSize: 14
                                            font.weight: Font.DemiBold
                                        }
                                        Text {
                                            // Layout.fillWidth: true
                                            Layout.alignment: Qt.AlignHCenter
                                            text: "Fine"
                                            color: Theme.white
                                            font.pixelSize: 14
                                            font.weight: Font.Medium
                                        }
                                        Text {
                                            // Layout.fillWidth: true
                                            Layout.alignment: Qt.AlignHCenter
                                            text: "Coarse"
                                            color: Theme.white
                                            font.pixelSize: 14
                                            font.weight: Font.Medium
                                        }
                                        Text {
                                            Layout.leftMargin: 20
                                            text: "Temporary"
                                            color: Theme.white
                                            font.pixelSize: 14
                                            font.weight: Font.Medium
                                        }
                                        SettingComponents.SpinBox {
                                            onValueChanged: decksTab.dirty = true
                                            id: adjustmentButtonsTemporaryFineInput
                                            Layout.fillWidth: true
                                            Layout.alignment: Qt.AlignHCenter
                                            Layout.leftMargin: 20
                                            Layout.rightMargin: 20
                                            realValue: 2
                                            precision: 2
                                            min: 0.01
                                            max: 10
                                            suffix: "%"
                                        }
                                        SettingComponents.SpinBox {
                                            onValueChanged: decksTab.dirty = true
                                            id: adjustmentButtonsTemporaryCoarseInput
                                            Layout.fillWidth: true
                                            Layout.alignment: Qt.AlignHCenter
                                            Layout.leftMargin: 20
                                            Layout.rightMargin: 20
                                            realValue: 4
                                            precision: 2
                                            min: 0.01
                                            max: 10
                                            suffix: "%"
                                        }
                                        Text {
                                            Layout.leftMargin: 20
                                            text: "Permanent"
                                            color: Theme.white
                                            font.pixelSize: 14
                                            font.weight: Font.Medium
                                        }
                                        SettingComponents.SpinBox {
                                            onValueChanged: decksTab.dirty = true
                                            id: adjustmentButtonsPermanentFineInput
                                            Layout.fillWidth: true
                                            Layout.alignment: Qt.AlignHCenter
                                            Layout.leftMargin: 20
                                            Layout.rightMargin: 20
                                            realValue: 0.05
                                            precision: 2
                                            min: 0.01
                                            max: 10
                                            suffix: "%"
                                        }
                                        SettingComponents.SpinBox {
                                            onValueChanged: decksTab.dirty = true
                                            id: adjustmentButtonsPermanentCoarseInput
                                            Layout.fillWidth: true
                                            Layout.alignment: Qt.AlignHCenter
                                            Layout.leftMargin: 20
                                            Layout.rightMargin: 20
                                            realValue: 0.5
                                            precision: 2
                                            min: 0.01
                                            max: 10
                                            suffix: "%"
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

                                            horizontalAlignment: Text.AlignLeft
                                            verticalAlignment: Text.AlignVCenter

                                            text: parent.label
                                            color: Theme.white
                                            font.pixelSize: 14
                                            font.weight: Font.Medium
                                        }
                                    }
                                    SettingComponents.Slider {
                                        onValueChanged: decksTab.dirty = true
                                        id: rampingSensitivityInput
                                        width: 400
                                        min: 100
                                        max: 2500
                                        wheelStep: 50
                                        value: 250
                                    }
                                }
                            }
                        }
                    }
                }
                Item {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }
            }
        }
    }

    Item {
        id: buttonActions
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.leftMargin: 14
        anchors.rightMargin: 14
        height: 20
        SettingComponents.FormButton {
            anchors.left: parent.left
            text: "Reset"
            opacity: enabled ? 1.0 : 0.5
            backgroundColor: "#7D3B3B"
            activeColor: "#999999"
            onPressed: {
                switch (root.selectedIndex) {
                    case 0:
                        root.resetInterface()
                        break;
                    case 1:
                        root.resetWaveform()
                        break;
                    case 0:
                        root.resetDeck()
                        break;
                }
            }
        }
        Row {
            spacing: 10
            anchors.right: parent.right
            Text {
                Layout.alignment: Qt.AlignVCenter
                Layout.rightMargin: 16
                id: errorMessage
                text: ""
                color: "#7D3B3B"
            }
            SettingComponents.FormButton {
                enabled: root.selectedIndex == 0 && themeColorTab.dirty || root.selectedIndex == 1 && waveformTab.dirty || root.selectedIndex == 2 && decksTab.dirty
                text: "Cancel"
                opacity: enabled ? 1.0 : 0.5
                backgroundColor: "#3F3F3F"
                activeColor: "#999999"
                onPressed: {
                    switch (root.selectedIndex) {
                        case 0:
                            root.loadInterface()
                            break;
                        case 1:
                            root.loadWaveform()
                            break;
                        case 2:
                            root.loadDeck()
                            break;
                    }
                }
            }
            SettingComponents.FormButton {
                enabled: root.selectedIndex == 0 && themeColorTab.dirty || root.selectedIndex == 1 && waveformTab.dirty || root.selectedIndex == 2 && decksTab.dirty
                text: "Save"
                opacity: enabled ? 1.0 : 0.5
                backgroundColor: root.hasChanges ? "#3a60be" : "#3F3F3F"
                activeColor: "#999999"
                onPressed: {
                    errorMessage.text = ""
                    switch (root.selectedIndex) {
                        case 0:
                            root.saveInterface()
                            break;
                        case 1:
                            root.saveWaveform()
                            break;
                        case 2:
                            root.saveDeck()
                            break;
                    }
                }
            }
        }
    }

    function resetInterface() {
    }

    function resetWaveform() {
    }

    function resetDeck() {
    }

    function loadInterface() {
        // Interface tab
        // skinInput.value =
        // colorInput.value =
        // layoutInput.value =
        tooltipsInput.selected = tooltipsInput.options[Mixxx.Config.libraryTooltips]
        disableScreensaverInput.selected = disableScreensaverInput.options[Mixxx.Config.libraryInhibitScreensaver]
        startFullscreenInput.selected = Mixxx.Config.configStartInFullscreenKey ? "on" : "off"
        autoHideMenuBarInput.selected = Mixxx.Config.libraryHideMenuBar ? "on" : "off"
        searchCompletionInput.selected = Mixxx.Config.libraryEnableSearchCompletions ? "on" : "off"
        searchHistoryKeyboardInput.selected = Mixxx.Config.libraryEnableSearchHistoryShortcuts ? "on" : "off"
        bpmPrecisionInput.value = Mixxx.Config.libraryBpmColumnPrecision
        libraryRowHeightInput.value = Mixxx.Config.libraryRowHeight
        trackPaletteComboBox.currentIndex = trackPaletteComboBox.model.indexOf(Mixxx.Config.configTrackColorPalette)
        hotcuePaletteComboBox.currentIndex = hotcuePaletteComboBox.model.indexOf(Mixxx.Config.configHotcueColorPalette)
        keyPaletteInput.selected = Mixxx.Config.configKeyColorsEnabled ? "on" : "off"
        keyPaletteComboBox.currentIndex = keyPaletteComboBox.model.indexOf(Mixxx.Config.configKeyColorPalette)

        colorPane.hotcuePaletteColorIndex = Mixxx.Config.controlHotcueDefaultColorIndex
        console.log(colorPane.hotcuePaletteColorIndex, Mixxx.Config.controlHotcueDefaultColorIndex)
        if (colorPane.hotcuePaletteColorIndex < 0) colorPane.hotcuePaletteColorIndex = colorPane.defaultPalette.length - 1
        hotcuePaletteInput.currentIndex = colorPane.hotcuePaletteColorIndex

        colorPane.loopPaletteColorIndex = Mixxx.Config.controlLoopDefaultColorIndex
        if (colorPane.loopPaletteColorIndex < 0) colorPane.loopPaletteColorIndex = colorPane.defaultPalette.length - 2
        loopPaletteInput.currentIndex = colorPane.loopPaletteColorIndex
        // Depends of #13216
        colorPane.jumpPaletteColorIndex = colorPane.defaultPalette.length - 3
        jumpPaletteInput.currentIndex = colorPane.jumpPaletteColorIndex
        themeColorTab.dirty = false
    }
    function loadWaveform() {
        endOfTrackWarningInput.value = Mixxx.Config.waveformEndOfTrackWarningTime
        beatGridAlphaInput.value = Mixxx.Config.waveformBeatGridAlpha
        defaultZoomInput.value = Mixxx.Config.waveformDefaultZoom * 10
        synchronizeAllZoomLevelInput.selected = Mixxx.Config.waveformZoomSynchronization ? "on" : "off"
        normaliseWaveformOverviewLevelInput.selected = Mixxx.Config.waveformOverviewNormalized ? "on" : "off"
        playMarkerPositionInput.value = Mixxx.Config.waveformPlayMarkerPosition * 100
        untilMarkShowTimeInput.load(Mixxx.Config.waveformUntilMarkShowTime)
        untilMarkShowBeatsInput.load( Mixxx.Config.waveformUntilMarkShowBeats)
        untilMarkAlignInput.load(Mixxx.Config.waveformUntilMarkAlign)
        untilMarkTextPointSizeInput.currentIndex = untilMarkTextPointSizeInput.fontSizes.includes(Mixxx.Config.waveformUntilMarkTextPointSize) ? untilMarkTextPointSizeInput.fontSizes.indexOf(Mixxx.Config.waveformUntilMarkTextPointSize) : 0
        visualGainAllInput.value = Mixxx.Config.waveformVisualGainAll * 100
        visualGainLowInput.value = Mixxx.Config.waveformVisualGainLow * 100
        visualGainMediumInput.value = Mixxx.Config.waveformVisualGainMedium * 100
        visualGainHighInput.value = Mixxx.Config.waveformVisualGainHigh * 100
        waveformDropdown.currentIndex = waveformDropdown.indices.indexOf(Mixxx.Config.waveformType)
        waveformOptionHighDetailsInput.selected = Mixxx.Config.waveformOptions & Mixxx.WaveformDisplay.Options.SplitStereoSignal ? "on" : "off"
        waveformOptionStereoInput.selected = Mixxx.Config.waveformOptions & Mixxx.WaveformDisplay.Options.HighDetail ? "on" : "off"
        waveformTab.dirty = false
    }
    function loadDeck() {
        // Decks tab
        cueModeInput.currentIndex = Mixxx.Config.controlCueDefault
        setIntroStartToMainCueInput.selected = Mixxx.Config.controlSetIntroStartAtMainCue ? "on" : "off"
        trackTimeDisplayInput.selected = trackTimeDisplayInput.options[Mixxx.Config.controlPositionDisplay]
        doublePressLoadToCloneInput.selected = Mixxx.Config.controlCloneDeckOnLoadDoubleTap ? "on" : "off"
        trackLoadPointInput.currentIndex = Mixxx.Config.controlCueRecall
        loadingTrackWhenPlayingInput.selected = loadingTrackWhenPlayingInput.options[Mixxx.Config.controlLoadWhenDeckPlaying]
        resetOnTrackLoadInput.selected = resetOnTrackLoadInput.options[Mixxx.Config.controlSpeedAutoReset]
        sliderRangeInput.currentIndex = sliderRangeInput.values.indexOf(Mixxx.Config.controlRateRange)
        if (sliderRangeInput.currentIndex === -1) {
            sliderRangeInput.values.push(Mixxx.Config.controlRateRange)
            sliderRangeInput.model.push(`${Mixxx.Config.controlRateRange}%`)
            sliderRangeInput.currentIndex = sliderRangeInput.model.length - 1
        }
        syncModeInput.selected = syncModeInput.options[Mixxx.Config.bpmSyncLockAlgorithm]
        sliderOrientationInput.selected = Mixxx.Config.controlRateDir === -1 ? "down" : "up"
        keylockModeInput.selected = keylockModeInput.options[Mixxx.Config.controlKeylockMode]
        keyunlockModeInput.selected = keyunlockModeInput.options[Mixxx.Config.controlKeyunlockMode]
        pitchBendBehaviourInput.selected = pitchBendBehaviourInput.options[Mixxx.Config.controlPitchBendBehaviour]
        adjustmentButtonsTemporaryCoarseInput.value = Mixxx.Config.controlRateTempCoarse * 100
        adjustmentButtonsTemporaryFineInput.value = Mixxx.Config.controlRateTempFine * 100
        adjustmentButtonsPermanentCoarseInput.value = Mixxx.Config.controlRatePermCoarse * 100
        adjustmentButtonsPermanentFineInput.value = Mixxx.Config.controlRatePermFine * 100
        rampingSensitivityInput.value = Mixxx.Config.controlRateRampSensitivity
        decksTab.dirty = false
    }

    function load() {
        loadInterface()
        loadWaveform()
        loadDeck()
        errorMessage.text = ""
    }

    function saveInterface() {
        // Interface tab
        // skinInput.value =
        // colorInput.value =
        // layoutInput.value =
        Mixxx.Config.libraryTooltips = tooltipsInput.options.indexOf(tooltipsInput.selected)
        Mixxx.Config.libraryInhibitScreensaver = disableScreensaverInput.options.indexOf(disableScreensaverInput.selected)
        Mixxx.Config.configStartInFullscreenKey = startFullscreenInput.enabled
        Mixxx.Config.libraryHideMenuBar = autoHideMenuBarInput.enabled
        Mixxx.Config.libraryEnableSearchCompletions = searchCompletionInput.enabled
        Mixxx.Config.libraryEnableSearchHistoryShortcuts = searchHistoryKeyboardInput.enabled
        Mixxx.Config.libraryBpmColumnPrecision = bpmPrecisionInput.value
        Mixxx.Config.libraryRowHeight = libraryRowHeightInput.value
        Mixxx.Config.configTrackColorPalette = trackPaletteComboBox.model[trackPaletteComboBox.currentIndex]
        Mixxx.Config.configHotcueColorPalette = hotcuePaletteComboBox.model[hotcuePaletteComboBox.currentIndex]
        Mixxx.Config.configKeyColorsEnabled = keyPaletteInput.enabled
        Mixxx.Config.configKeyColorPalette = keyPaletteComboBox.model[keyPaletteComboBox.currentIndex]
        // keyPaletteInput.value
        Mixxx.Config.controlHotcueDefaultColorIndex = hotcuePaletteInput.currentIndex
        Mixxx.Config.controlLoopDefaultColorIndex = loopPaletteInput.currentIndex
        // jumpPaletteInput.value =
        loadInterface()
    }

    function saveWaveform() {
        Mixxx.Config.waveformPlayMarkerPosition = waveformTab.playMarkerPosition
        Mixxx.Config.waveformDefaultZoom = waveformTab.defaultZoom
        Mixxx.Config.waveformBeatGridAlpha = waveformTab.beatgridOpacity
        Mixxx.Config.waveformUntilMarkShowTime = waveformTab.untilMarkShowTime
        Mixxx.Config.waveformUntilMarkShowBeats = waveformTab.untilMarkShowBeats
        Mixxx.Config.waveformUntilMarkAlign = waveformTab.playMarkerAlign
        Mixxx.Config.waveformUntilMarkTextPointSize = waveformTab.playMarkerTextSize

        Mixxx.Config.waveformVisualGainAll = visualGainAllInput.value / 100
        Mixxx.Config.waveformVisualGainLow = visualGainLowInput.value / 100
        Mixxx.Config.waveformVisualGainMedium = visualGainMediumInput.value / 100
        Mixxx.Config.waveformVisualGainHigh = visualGainHighInput.value / 100

        Mixxx.Config.waveformType = waveformDropdown.indices[waveformDropdown.currentIndex]
        Mixxx.Config.waveformOptions = (waveformOptionHighDetailsInput.enabled ? Mixxx.WaveformDisplay.Options.HighDetail : 0) + (waveformOptionStereoInput.enabled ? Mixxx.WaveformDisplay.Options.SplitStereoSignal : 0)
        loadWaveform()
    }

    function saveDeck() {
        Mixxx.Config.controlCueDefault = cueModeInput.currentIndex
        Mixxx.Config.controlSetIntroStartAtMainCue = setIntroStartToMainCueInput.enabled
        Mixxx.Config.controlPositionDisplay = trackTimeDisplayInput.options.indexOf(trackTimeDisplayInput.selected)
        Mixxx.Config.controlCloneDeckOnLoadDoubleTap = doublePressLoadToCloneInput.enabled
        Mixxx.Config.controlCueRecall = trackLoadPointInput.currentIndex
        Mixxx.Config.controlLoadWhenDeckPlaying = loadingTrackWhenPlayingInput.options.indexOf(loadingTrackWhenPlayingInput.selected)
        Mixxx.Config.controlSpeedAutoReset = resetOnTrackLoadInput.options.indexOf(resetOnTrackLoadInput.selected)
        for (let i = 0; i < deckRateRange.count; i++) {
            deckRateRange.objectAt(i).value = sliderRangeInput.values[sliderRangeInput.currentIndex]
        }
        Mixxx.Config.controlRateRange = sliderRangeInput.values[sliderRangeInput.currentIndex]
        Mixxx.Config.bpmSyncLockAlgorithm = syncModeInput.options.indexOf(syncModeInput.selected)
        for (let i = 0; i < deckRateDirection.count; i++) {
            deckRateDirection.objectAt(i).value = sliderOrientationInput.selected === "down" ? -1 : 1
        }
        Mixxx.Config.controlRateDir = sliderOrientationInput.selected === "down" ? -1 : 1
        Mixxx.Config.controlKeylockMode = keylockModeInput.options.indexOf(keylockModeInput.selected)
        Mixxx.Config.controlKeyunlockMode = keyunlockModeInput.options.indexOf(keyunlockModeInput.selected)
        Mixxx.Config.controlPitchBendBehaviour = pitchBendBehaviourInput.options.indexOf(pitchBendBehaviourInput.selected)
        Mixxx.Config.controlRateTempCoarse = adjustmentButtonsTemporaryCoarseInput.value / 100
        Mixxx.Config.controlRateTempFine = adjustmentButtonsTemporaryFineInput.value / 100
        Mixxx.Config.controlRatePermCoarse = adjustmentButtonsPermanentCoarseInput.value / 100
        Mixxx.Config.controlRatePermFine = adjustmentButtonsPermanentFineInput.value / 100
        Mixxx.Config.controlRateRampSensitivity = rampingSensitivityInput.value
        loadDeck()
    }
}
