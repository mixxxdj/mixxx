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
    tabs: ["theme & colour", "waveform", "decks"]
    selectedIndex: 2

    property bool dirty: false

    label: "Interface"

    Item {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: buttonActions.top
        anchors.bottomMargin: 18

        Mixxx.SettingGroup {
            label: "Theme & Colour"
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
                            width: parent.width - 180
                            height: width / 16 * 9
                            color: '#343434'
                            Skin.Button {
                                anchors.horizontalCenter: parent.horizontalCenter
                                anchors.bottom: parent.bottom
                                anchors.bottomMargin: 20
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
                                        inactiveColor: Theme.darkGray4
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
                                        inactiveColor: Theme.darkGray4
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
                                    RatioChoice {
                                        inactiveColor: Theme.darkGray4
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
                    label: "Colours"
                    implicitHeight: colourColumn.height
                    Layout.fillWidth: true
                    Column {
                        id: colourColumn
                        anchors.left: parent.left
                        anchors.right: parent.right
                        Text {
                            text: "Colours"
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
                            implicitHeight: colourPane.implicitHeight + 20
                            anchors.left: parent.left
                            anchors.right: parent.right
                            GridLayout {
                                id: colourPane
                                anchors.fill: parent
                                anchors.topMargin: 10
                                anchors.bottomMargin: 10
                                anchors.leftMargin: 3
                                anchors.rightMargin: 3
                                rowSpacing: 8
                                columnSpacing: 20
                                columns: 2

                                readonly property var defaultPalette: Mixxx.Config.getHotcueColorPalette()

                                RowLayout {
                                    Layout.preferredWidth: (colourPane.width - 56) * 0.5
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
                                        id: trackPaletteComboBox
                                        model: [
                                                "Serato DJ Track Metadata Hotcue Colors",
                                                "Mixxx Track Colors",
                                                "Rekordbox Track Colors",
                                                "Serato DJ Pro Track Colors",
                                                "Traktor Pro Track Colors",
                                                "VirtualDJ Track Colors",
                                        ]
                                    }
                                }
                                RowLayout {
                                    Layout.preferredWidth: (colourPane.width - 56) * 0.5
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
                                        id: hotcuePaletteComboBox
                                        model: [
                                                "Mixxx Hotcue Colors",
                                                "Serato DJ Pro Hotcue Colors",
                                                "Rekordbox COLD1 Hotcue Colors",
                                                "Rekordbox COLD2 Hotcue Colors",
                                                "Rekordbox COLORFUL Hotcue Colors"
                                        ]
                                    }
                                }
                                RowLayout {
                                    Layout.preferredWidth: (colourPane.width - 56) * 0.5
                                    Layout.leftMargin: 14
                                    Layout.rightMargin: 14
                                    Mixxx.SettingParameter {
                                        label: "Key palette"
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
                                        inactiveColor: Theme.darkGray4
                                        options: [
                                                  "on",
                                                  "off"
                                        ]
                                    }
                                }
                                Rectangle {
                                    Layout.preferredWidth: (colourPane.width - 56) * 0.5
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
                                                label: "Hotcue default colour palette"
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
                                                selectedColor: colourPane.defaultPalette[colourPane.defaultPalette.length-1]
                                            }
                                        }
                                        ColumnLayout {
                                            Layout.fillHeight: true
                                            spacing: 15
                                            Mixxx.SettingParameter {
                                                label: "Loop default colour palette"
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
                                                selectedColor: colourPane.defaultPalette[colourPane.defaultPalette.length-2]
                                            }
                                        }
                                        ColumnLayout {
                                            id: jumpDefaultColor
                                            Layout.fillHeight: true
                                            spacing: 15
                                            Mixxx.SettingParameter {
                                                label: "Jump default colour palette"
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
                                                selectedColor: colourPane.defaultPalette[colourPane.defaultPalette.length-3]
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
            id: waveformTab
            label: "Waveform"
            visible: root.selectedIndex == 1
            anchors.fill: parent

            onActivated: {
                root.selectedIndex = 1;
            }

            readonly property real playMarkerPosition: playMarkerPositionInput.value / 100
            readonly property real defaultZoom: defaultZoomInput.value / 10
            readonly property real beatgridOpacity: beatgridOpacityInput.value / 100
            property alias playMarkerShowTime: playMarkerShowTimeInput.enabled
            property alias playMarkerShowBeats: playMarkerShowBeatsInput.enabled
            readonly property var playMarkerAlign: playMarkerAlignInput.selected == "center" ? Qt.AlignHCenter : playMarkerAlignInput.selected == "top" ? Qt.AlignTop : Qt.AlignBottom
            readonly property double playMarkerTextSize: parseInt(playMarkerTextSizeInput.currentText.split(' ')[0])

            readonly property double globalGain: globalGainInput.value / 100
            readonly property double lowGain: lowGainInput.value / 100
            readonly property double middleGain: middleGainInput.value / 100
            readonly property double highGain: highGainInput.value / 100

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
                                id: beatgridOpacityInput
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
                            id: waveformDropdown

                            Layout.fillWidth: true
                            Layout.preferredHeight: 102

                            model: ["Filtered", "HSV", "RGB", "Simple", "Stacked"]
                            currentIndex: 2

                            property var track: waveformDropdown.player.isLoaded ? waveformDropdown.player.currentTrack : Mixxx.Library.model.getTrack(Mixxx.Library.model.length * Math.random())
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
                                highColor: waveformDropdown.midColor

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
                                interval: 25; running: !waveformDropdown.player.isLoaded; repeat: true
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

                                            untilMark.showTime: waveformTab.playMarkerShowTime
                                            untilMark.showBeats: waveformTab.playMarkerShowBeats
                                            untilMark.align: waveformTab.playMarkerAlign
                                            untilMark.textSize: waveformTab.playMarkerTextSize
                                            untilMark.defaultNextMarkPosition: waveformDropdown.player.isLoaded ? -1 : waveformDropdown.track.duration * waveformDropdown.track.sampleRate * 2
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

                                            renderers: [
                                                        (waveformDropdown.currentIndex == 0 ? waveformDropdown.filteredRenderer :
                                                            waveformDropdown.currentIndex == 1 ? waveformDropdown.hsvRenderer :
                                                            waveformDropdown.currentIndex == 2 ? waveformDropdown.rgbRenderer :
                                                            waveformDropdown.currentIndex == 3 ? waveformDropdown.simpleRenderer :
                                                            waveformDropdown.stackedRenderer),
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
                                        id: playMarkerShowBeatsInput
                                        inactiveColor: Theme.darkGray4
                                        readonly property bool enabled: selected == "on"
                                        options: [
                                                  "on",
                                                  "off"
                                        ]
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
                                        id: playMarkerAlignInput
                                        options: [
                                                  "top",
                                                  "center",
                                                  "bottom"
                                        ]
                                        selected: "center"
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
                                        id: playMarkerShowTimeInput
                                        inactiveColor: Theme.darkGray4
                                        readonly property bool enabled: selected == "on"
                                        options: [
                                                  "on",
                                                  "off"
                                        ]
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
                                        id: playMarkerTextSizeInput
                                        model: [
                                                "10 pt",
                                                "12 pt",
                                                "15 pt",
                                                "18 pt",
                                                "24 pt",
                                                "32 pt",
                                                "48 pt"
                                        ]
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
                                id: globalGainInput
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
                                        Layout.fillWidth: true
                                        id: lowGainInput
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
                                        Layout.fillWidth: true
                                        id: middleGainInput
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
                                        Layout.fillWidth: true
                                        id: highGainInput
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
            label: "Decks"
            visible: root.selectedIndex == 2
            anchors.fill: parent

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
                            model: [
                                    "Intro start",
                                    "Main cue",
                                    "First hotcue",
                                    "First sound",
                                    "Beginning of track",
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
                            options: [
                                      "reject",
                                      "stopped only",
                                      "allow"
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
                                        inactiveColor: Theme.darkGray4
                                        options: [
                                                  "none",
                                                  "key",
                                                  "tempo",
                                                  "both"
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
                                                text: "Define which end of the slider while increase the pitch"
                                                color: Theme.white
                                                font.pixelSize: 11
                                                font.italic: true
                                                font.weight: Font.Thin
                                            }
                                        }
                                    }
                                    RatioChoice {
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
                                        inactiveColor: Theme.darkGray4
                                        options: [
                                                  "smooth ramping",
                                                  "abrupt jump"
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
                                        Layout.leftMargin: 20
                                        Layout.rightMargin: 20
                                        columns: 3
                                        Item {
                                            Layout.fillWidth: true
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
                                            text: "Temporary"
                                            color: Theme.white
                                            font.pixelSize: 14
                                            font.weight: Font.Medium
                                        }
                                        SettingComponents.SpinBox {
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
                                            text: "Permanent"
                                            color: Theme.white
                                            font.pixelSize: 14
                                            font.weight: Font.Medium
                                        }
                                        SettingComponents.SpinBox {
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
                root.reset()
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
                enabled: root.dirty
                text: "Cancel"
                opacity: enabled ? 1.0 : 0.5
                backgroundColor: "#3F3F3F"
                activeColor: "#999999"
                onPressed: {
                    root.selectedController = null
                    root.load()
                }
            }
            SettingComponents.FormButton {
                enabled: root.dirty
                text: "Save"
                opacity: enabled ? 1.0 : 0.5
                backgroundColor: root.hasChanges ? "#3a60be" : "#3F3F3F"
                activeColor: "#999999"
                onPressed: {
                    errorMessage.text = ""
                    root.save()
                }
            }
        }
    }

    function reset() {
    }

    function load() {
        root.dirty = false
        errorMessage.text = ""
    }

    function save() {
        root.dirty = false
        errorMessage.text = ""
    }
}
