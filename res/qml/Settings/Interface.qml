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
